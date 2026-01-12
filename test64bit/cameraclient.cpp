#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QtEndian>   // qToBigEndian / qFromBigEndian
#include "CameraClient.h"
#include <QDataStream>
#include <QBuffer>
#include "CameraImageProvider.h"
#include <QImage>
#include <QThread>
#include "appconfig.h"
#include <QCoreApplication>
#include <QDir>
#include <windows.h>
#include <QTimer>
#include "CaptureFlow.h"
#include "StartPreviewFlow.h"

static constexpr int HEADER_LEN = 16;

static constexpr quint16 PROTO_VER = 1;
static constexpr quint16 MSG_COMMAND_REQUEST = 1;

// TODO: 定义你的推帧消息类型（例如你 server 的 FRAME_TYPE_PREVIEW）
//static constexpr quint16 MSG_FRAME_PREVIEW  = 0x1001;

static quint64 ntohll(quint64 v)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    quint32 hi = ntohl(quint32(v >> 32));
    quint32 lo = ntohl(quint32(v & 0xFFFFFFFFULL));
    return (quint64(lo) << 32) | hi;
#else
    return v;
#endif
}


CameraClient::CameraClient(CameraImageProvider* provider, QObject* parent)
    : provider_(provider)
{
    worker_ = new SocketWorker();
    worker_->moveToThread(&socketThread_);
    connect(&socketThread_, &QThread::finished, worker_, &QObject::deleteLater);
    socketThread_.start();

    // ✅ 就 4 条：和你原来几乎一一对应
    connect(worker_, &SocketWorker::connected,    this, &CameraClient::onConnected, Qt::QueuedConnection);
    connect(worker_, &SocketWorker::disconnected, this, &CameraClient::onDisconnected, Qt::QueuedConnection);
    connect(worker_, &SocketWorker::readyReadData,this, &CameraClient::onReadyRead, Qt::QueuedConnection);
    connect(worker_, &SocketWorker::errorOccurredStr, this, &CameraClient::onErrorOccurred, Qt::QueuedConnection);
    connect(worker_, &SocketWorker::exitFinished,
            this, [this](bool ok){
                emit log(ok ? "exit ok" : "exit timeout");

                // 1) 唤醒等待（给析构/退出流程用）
                exitDone_.store(true, std::memory_order_release);
                {
                    QMutexLocker lk(&exitMtx_);
                    exitCv_.wakeAll();
                }

                // 2) 业务收尾：让 worker 清理 socket（异步，不阻塞 UI）
                QMetaObject::invokeMethod(worker_, "shutdown", Qt::QueuedConnection);

                connected_ = false;

                // 3) 请求线程退出（不 wait！wait 放到析构里）
                socketThread_.quit();
            },
            Qt::QueuedConnection);

}

CameraClient::~CameraClient()
{
    if (exitInProgress_.load(std::memory_order_acquire) &&
        !exitDone_.load(std::memory_order_acquire))
    {
        QMutexLocker lk(&exitMtx_);
        exitCv_.wait(&exitMtx_, 1200); // 最多等 1.2s，避免卡死
    }


    // 让 worker 线程去断开（不等待）
    if (socketThread_.isRunning()) {
        socketThread_.quit();
        socketThread_.wait();   // ✅ 关键：等线程真的退出
    }

}

void CameraClient::setCustomerID(const QString &strCustomerID)
{
    CustomerID_ = strCustomerID;
}

void CameraClient::startup()
{
    // 1) 启动 worker 线程（如果你在构造函数里已经 start 了，可省略）
    if (!socketThread_.isRunning())
        socketThread_.start();

    // 2) 通过 queued 方式，让 worker 在自己的线程里创建 QTcpSocket 并 connect
    QMetaObject::invokeMethod(worker_, "startup",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QStringLiteral("127.0.0.1")),
                              Q_ARG(quint16, 12345));

}

void CameraClient::openCamera() {
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "open";
    obj["series"] = AppConfig::instance().CameraSeries();

    sendCommandRequest(obj);
}

void CameraClient::startPreview() {
    if (!connected()) { emit log("Not connected."); return; }

    auto* flow = new StartPreviewFlow(this,this);
    connect(flow, &StartPreviewFlow::finished, this, [this](bool ok, const QString& msg){
        emit log(msg);
        if(ok)
        {
            previewOn_ = true;
            emit previewOnChanged();
        }
    });
    flow->start();
}

void CameraClient::capture()
{
    if (!connected()) { emit log("Not connected."); return; }
    if (!previewOn()) { emit log("Not previewOn."); return; }

    left_pics_.clear();
    right_pics_.clear();
    emit left_picsChanged();
    emit right_picsChanged();


    auto* flow = new CaptureFlow(this,CustomerID_,AppDb::instance().GetNextGroupID(CustomerID_),this);
    connect(flow, &CaptureFlow::finished, this, [this](bool ok, const QString& msg){
        emit log(msg);
    });
    flow->start();
}

void CameraClient::stopPreview()
{
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "stoppreview";

    sendCommandRequest(obj);

    previewOn_ = false;
    emit previewOnChanged();
}

void CameraClient::save()
{
    stopPreview();

    left_pics_.clear();
    right_pics_.clear();
    emit left_picsChanged();
    emit right_picsChanged();
}

void CameraClient::cancel()
{
    stopPreview();
    left_pics_.clear();
    right_pics_.clear();
    emit left_picsChanged();
    emit right_picsChanged();
}


void CameraClient::closeCamera() {
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "exit";

    sendCommandRequest(obj);
    previewOn_ = false;


}


bool CameraClient::sendMessage(quint16 msgType, quint32 requestId, const QByteArray& payload)
{
    // body = [MessageHeader][payload]
    const quint32 bodyLen = 8u + (quint32)payload.size(); // MessageHeader 固定 8 字节
    QByteArray buf;
    buf.resize(4 + (int)bodyLen);

    // 1) 写 4 字节 bodyLen（大端）
    {
        quint32 beBodyLen = qToBigEndian(bodyLen);
        memcpy(buf.data(), &beBodyLen, 4);
    }

    // 2) 写 MessageHeader（大端）
    {
        quint16 beMsgType = qToBigEndian(msgType);
        quint16 beVer     = qToBigEndian((quint16)PROTO_VER);
        quint32 beReqId   = qToBigEndian(requestId);

        memcpy(buf.data() + 4, &beMsgType, 2);
        memcpy(buf.data() + 6, &beVer,     2);
        memcpy(buf.data() + 8, &beReqId,   4);
    }

    // 3) 写 payload
    if (!payload.isEmpty()) {
        memcpy(buf.data() + 12, payload.constData(), payload.size());
    }

    // 4) ✅ 投递到 socket 线程写入
    bool accepted = false;
    QMetaObject::invokeMethod(worker_, "send",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, buf));


    return true;
}

quint32 CameraClient::sendCommandRequest2(const QJsonObject& obj, RespHandler onResp)
{
    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    quint32 reqId = nextReqId_.fetch_add(1, std::memory_order_relaxed);
    if (!sendMessage(MSG_COMMAND_REQUEST, reqId, json)) {
        emit log("sendCommandRequest failed: not connected?");
        return 0;
    }

    QString cmdSend = obj.value("cmd").toString();
    if (cmdSend == "exit") {
        exitInProgress_.store(true, std::memory_order_release);
        exitDone_.store(false, std::memory_order_release);
        QMetaObject::invokeMethod(worker_, "beginExitWait",
                                  Qt::QueuedConnection,
                                  Q_ARG(quint32, reqId),
                                  Q_ARG(int, 1000));
    }

    pending_[reqId] = [this, reqId, onResp](const QJsonObject& resp) mutable {
        QString cmd = resp.value("cmd").toString();
        QString result = resp.value("result").toString();
        emit log(QString("RESP cmd=%1 result=%2").arg(cmd, result));

        // 你的原逻辑：exit 特殊处理
        if (cmd == "exit") {
            QMetaObject::invokeMethod(worker_, "notifyExitResp",
                                      Qt::QueuedConnection,
                                      Q_ARG(quint32, reqId));
        }

        // ⭐ 调用调用者传入的回调
        if (onResp) onResp(resp);

        // 处理完就清理，避免 pending_ 越积越多
        pending_.remove(reqId);
    };

    return reqId;
}

quint32 CameraClient::sendCommandRequest(const QJsonObject& obj)
{
    // JSON -> UTF-8 bytes
    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    quint32 reqId = nextReqId_.fetch_add(1, std::memory_order_relaxed);
    if (!sendMessage(MSG_COMMAND_REQUEST, reqId, json)) {
        emit log("sendCommandRequest failed: not connected?");
        return 0;
    }


    // ⭐ 如果是 exit，提前告诉 worker：我要等这个 reqId
    QString cmd = obj.value("cmd").toString();
    if (cmd == "exit")
    {
        exitInProgress_.store(true, std::memory_order_release);
        exitDone_.store(false, std::memory_order_release);
        QMetaObject::invokeMethod(worker_, "beginExitWait",Qt::QueuedConnection,Q_ARG(quint32, reqId),Q_ARG(int, 1000));   //  1 秒超时

    }

    pending_[reqId] = [this,reqId](const QJsonObject& resp){
        QString cmd = resp.value("cmd").toString();
        QString result = resp.value("result").toString();
        emit log(QString("RESP  cmd=%1 result=%2 ").arg(cmd).arg(result));
        if (result != "OK")
        {
            return;
        }
        else if(cmd=="exit")
        {
            // ⭐ 通知 worker：exit response 已收到
            QMetaObject::invokeMethod(worker_, "notifyExitResp",Qt::QueuedConnection,Q_ARG(quint32, reqId));
        }
    };
    return reqId;
}

void CameraClient::onConnected() {
    emit log("Connected.");
    //emit connectedChanged();
    // 程序启动连接成功后，自动 open camera
    connected_ =true;
    openCamera();
}

void CameraClient::onDisconnected() {
    emit log("Disconnected.");
    rxBuf_.clear();
}

void CameraClient::onErrorOccurred(const QString& err) {
    qWarning() << "[CameraClient][SocketError]" << err;

    //emit socketError(err);

    //emit connectedChanged();
}

void CameraClient::onReadyRead(const QByteArray& chunk)
{
    rxBuf_.append(chunk);

    while (true) {
        // 1) 至少要有 4 字节长度
        if (rxBuf_.size() < 4) return;

        // 2) 读取 bodyLen（大端）
        quint32 beBodyLen = 0;
        memcpy(&beBodyLen, rxBuf_.constData(), 4);
        quint32 bodyLen = qFromBigEndian(beBodyLen);

        // 保护：防止异常长度导致内存炸
        if (bodyLen > 50 * 1024 * 1024) { // 50MB 你可按需改
            emit log("Bad bodyLen, drop connection.");
            rxBuf_.clear();
            return;
        }

        // 3) 半包：还没收齐 body
        if (rxBuf_.size() < 4 + (int)bodyLen) return;

        // 4) 拿到完整 body（不含前4字节）
        QByteArray body = rxBuf_.mid(4, bodyLen);
        rxBuf_.remove(0, 4 + (int)bodyLen);

        // 5) 解析 MessageHeader（8 字节）
        if (body.size() < 8) {
            emit log("Body too short for MessageHeader.");
            continue;
        }

        quint16 beMsgType = 0, beVer = 0;
        quint32 beReqId = 0;
        memcpy(&beMsgType, body.constData() + 0, 2);
        memcpy(&beVer,     body.constData() + 2, 2);
        memcpy(&beReqId,   body.constData() + 4, 4);

        quint16 msgType = qFromBigEndian(beMsgType);
        quint16 ver     = qFromBigEndian(beVer);
        quint32 reqId   = qFromBigEndian(beReqId);

        if (ver != 1) {
            emit log(QString("Unsupported protocol version=%1").arg(ver));
            continue;
        }

        QByteArray payload = body.mid(8);

        // 6) 分发处理
        switch (msgType) {
        case 2: // MSG_COMMAND_RESPONSE
            handleCommandResponse(reqId, payload);
            break;

        case 3: // MSG_FRAME_DATA
            handleFrameData(reqId, payload,bodyLen);
            break;

        case 4: // MSG_EVENT_NOTIFICATION
            handleEventNotification(payload);
            break;

        default:
            emit log(QString("Unknown msgType=%1, payloadLen=%2").arg(msgType).arg(payload.size()));
            break;
        }
    }
}

void CameraClient::handleEventNotification(const QByteArray& payload)
{

}

void CameraClient::handleFrameData(quint32 reqId, const QByteArray& payload, quint32 bodyLen)
{
    // 解析 FrameDataHeader（网络序->主机序）
    const char* p = payload.constData();
    FrameDataHeader fNet;

    memcpy(&fNet.frameType, p, 1);
    memcpy(&fNet.reserved1, p+1, 1);
    memcpy(&fNet.reserved2, p+2, 2);
    memcpy(&fNet.seq, p+4, 8);
    memcpy(&fNet.timestampMs, p+12, 8);
    memcpy(&fNet.width, p+20, 4);
    memcpy(&fNet.height, p+24, 4);
    memcpy(&fNet.pixelFormat, p+28, 4);
    memcpy(&fNet.dataLength, p+32, 4);

    FrameDataHeader fHost = fNet;
    fHost.seq         = ntohll(fNet.seq);
    fHost.timestampMs = ntohll(fNet.timestampMs);
    fHost.width       = ntohl(fNet.width);
    fHost.height      = ntohl(fNet.height);
    fHost.pixelFormat = ntohl(fNet.pixelFormat);
    fHost.dataLength  = ntohl(fNet.dataLength);
    // frameType 1字节无需转换
    // 计算 payload 起始与长度
    const int headerBytes = 8+36;
    const int payloadBytesByBody = int(bodyLen) - headerBytes;

    // 用 header 的 dataLength 做一致性校验
    if (payloadBytesByBody < 0 || quint32(payloadBytesByBody) != fHost.dataLength) {
        qWarning() << "Payload length mismatch. body says"
                   << payloadBytesByBody << "hdr says" << fHost.dataLength;
        return;
    }

    QByteArray imgData = payload.mid(36);
    // 拷贝 payload（图像数据）
    QImage img;
    if (!img.loadFromData(reinterpret_cast<const uchar*>(imgData.constData()), imgData.size())) {
        qWarning() << "loadFromData failed";
        return;
    }

    onFrameImageDecoded(img);
}

void CameraClient::handleCommandResponse(quint32 requestId, const QByteArray& payload)
{
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit log(QString("Bad JSON response, reqId=%1, err=%2")
                     .arg(requestId).arg(err.errorString()));
        return;
    }

    QJsonObject obj = doc.object();
    // QString cmd = obj.value("cmd").toString();
    // int result  = obj.value("result").toInt(-9999);
    // QString msg = obj.value("message").toString();

    // emit log(QString("RESP reqId=%1 cmd=%2 result=%3 msg=%4")
    //              .arg(requestId).arg(cmd).arg(result).arg(msg));

    // 如果你有 pending 回调，就在这里触发
    auto it = pending_.find(requestId);
    if (it != pending_.end()) {
        auto cb = it.value();
        pending_.erase(it);
        if (cb) cb(obj);
    }
}

void CameraClient::onFrameImageDecoded(const QImage& img)
{
    if (!provider_) return;
    provider_->setLatestImage(img);

    // 通知 QML 这张图“变了”
    ++frameToken_;
    emit frameTokenChanged();
}

QVariantList CameraClient::isos() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "iso");
}

QVariantList CameraClient::exposuretimes() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "exposuretime");
}

QVariantList CameraClient::apertures() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "aperture");

}
QVariantList CameraClient::wbs() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "wb");

}
QVariantList CameraClient::imageSizes() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "ImageSize");

}

QVariantList CameraClient::imageQualities() const
{
    return AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), "ImageQuality");

}

static int findIndexByValue(const QVariantList& list, const QVariant& target)
{
    for (int i = 0; i < list.size(); ++i) {
        const auto m = list[i].toMap();
        const QVariant v = m.value("value");

        if (v.toInt()==target.toInt())
             return i;
    }
    return -1;
}

QVariantList  CameraClient::settings()
{
    // 1) 取 T_CaptureSetting
    CaptureSetting s;
    AppDb::instance().loadCaptureSettingBySeries(AppConfig::instance().CameraSeries(),s);

    // 2) 定义输出项的“模板”（顺序就是你 QML settings 的顺序）
    struct RowDef {
        QString key;      // ConfigType
        QString title;    // 显示标题（你 settings 里的 text）
        QVariant raw;     // T_CaptureSetting 对应的值
    };

    const QList<RowDef> rows = {
        {"iso",          "RGB ISO",   s.rgb_iso},
        {"exposuretime", "RGB 快门",   s.rgb_exposureTime},
        {"aperture",     "RGB 光圈",   s.rgb_aperture},
        {"wb",           "RGB 白平衡", s.rgb_wb},

        {"iso",          "UV ISO",    s.uv_iso},
        {"exposuretime", "UV 快门",    s.uv_exposureTime},
        {"aperture",     "UV 光圈",    s.uv_aperture},
        {"wb",           "UV 白平衡",  s.uv_wb},

        {"iso",          "PL ISO",    s.pl_iso},
        {"exposuretime", "PL 快门",    s.pl_exposureTime},
        {"aperture",     "PL 光圈",    s.pl_aperture},
        {"wb",           "PL 白平衡",  s.pl_wb},

        {"iso",          "NPL ISO",   s.npl_iso},
        {"exposuretime", "NPL 快门",   s.npl_exposureTime},
        {"aperture",     "NPL 光圈",   s.npl_aperture},
        {"wb",           "NPL 白平衡", s.npl_wb},

        {"ImageSize",    "图片尺寸",    s.imageSize},
        {"ImageQuality", "图片质量",    s.imageQuality}
    };

    QVariantList  out;
    // 3) 为了避免重复查库：按 key 缓存候选列表（ISO/WB/...）
    QHash<QString, QVariantList> cache;

    auto getOptions = [&](const QString& key) -> const QVariantList& {
        auto it = cache.find(key);
        if (it != cache.end()) return it.value();
        cache.insert(key, AppDb::instance().cameraConfigList(AppConfig::instance().CameraSeries(), key));
        return cache[key];
    };

    // 4) 组装输出 settings
    out.reserve(rows.size());

    for (const auto& r : rows) {
        const QVariantList& opts = getOptions(r.key);
        const int idx = findIndexByValue(opts, r.raw);

        QVariantMap item;
        item.insert("key", r.key);
        item.insert("text", r.title);
        item.insert("curIndex", idx);

        if (idx >= 0) {
            const auto m = opts[idx].toMap();
            item.insert("value", m.value("text").toString()); // 给 UI 显示
            // 如果你希望 QML 侧也能拿到真实数值（用于回写/设置相机），建议一起带上：
            item.insert("rawValue", m.value("value"));
        } else {
            // 找不到映射就直接显示数值
            item.insert("value", r.raw.toString());
            item.insert("rawValue", r.raw);
        }

        out.push_back(item);
    }

    //applyCaptureSettingToCamera(s);

    return out;

}

QVariantList  CameraClient::left_pics() const
{
    return left_pics_;
}
QVariantList  CameraClient::right_pics() const
{
    return right_pics_;
}

void CameraClient::addleft(const QString& str)
{
    QString abs = QFileInfo(str).absoluteFilePath();
    QUrl url = QUrl::fromLocalFile(abs);

    left_pics_.append(url);
    emit left_picsChanged();
}

void CameraClient::addright(const QString& str)
{
    QString abs = QFileInfo(str).absoluteFilePath();
    QUrl url = QUrl::fromLocalFile(abs);
    right_pics_.append(url);
    emit right_picsChanged();
}

void CameraClient::onSettingChanged(int rowIndex, const QString& key, const QString& displayText,const QVariant& rawValue)
{
    Q_UNUSED(displayText);

    // 1) 按 rowIndex 映射到 T_CaptureSetting 字段名（最关键）
    static const QStringList fieldMap = {
        "RGB_ISO", "RGB_ExposureTime", "RGB_Aperture", "RGB_WB",
        "UV_ISO",  "UV_ExposureTime",  "UV_Aperture",  "UV_WB",
        "PL_ISO",  "PL_ExposureTime",  "PL_Aperture",  "PL_WB",
        "NPL_ISO", "NPL_ExposureTime", "NPL_Aperture", "NPL_WB",
        "ImageSize", "ImageQuality"
    };

    if (rowIndex < 0 || rowIndex >= fieldMap.size()) return;
    const QString col = fieldMap[rowIndex];

    // 2) 写回数据库：只更新这一列（只改动的地方）
    AppDb::instance().CaptureSettingUpdate(AppConfig::instance().CameraSeries(),col,rawValue.toInt());

    // 3) 同步设相机参数（按你的 key 或 col 判断）
    if(col.startsWith("RGB_") || col.startsWith("Image"))
    {
        if (!connected()) { emit log("Not connected."); return; }
        QJsonObject obj;
        obj["cmd"] = "online_setting";
        obj["key"] = key;
        obj["value"] = rawValue.toInt();
        obj["series"] = AppConfig::instance().CameraSeries();

        sendCommandRequest(obj);
    }
}
