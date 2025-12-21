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
#include <windows.h>
#include <tlhelp32.h>
#include <QThread>

static constexpr int HEADER_LEN = 16; // TODO: 改成你的真实 MessageHeader 长度

static constexpr quint16 PROTO_VER = 1;
static constexpr quint16 MSG_COMMAND_REQUEST = 1;

// TODO: 定义你的推帧消息类型（例如你 server 的 FRAME_TYPE_PREVIEW）
static constexpr quint16 MSG_FRAME_PREVIEW  = 0x1001;

bool isProcessRunning(const wchar_t* exeName)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, exeName) == 0) {
                CloseHandle(snap);
                return true;
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return false;
}

bool startProcess(const wchar_t* exePath)
{
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    BOOL ok = CreateProcessW(
        exePath,
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
        );

    if (!ok)
        return false;

    // 不需要等待它结束，只是启动
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

bool waitForServerReady(const QString& host, quint16 port, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        QTcpSocket sock;
        sock.connectToHost(host, port);
        if (sock.waitForConnected(300)) {
            sock.disconnectFromHost();
            return true;
        }
        QThread::msleep(200);
    }
    return false;
}


CameraClient::CameraClient(CameraImageProvider* provider, QObject* parent)
    : QObject(parent)
    ,provider_(provider)
{
    connect(&socket_, &QTcpSocket::connected, this, &CameraClient::onConnected);
    connect(&socket_, &QTcpSocket::disconnected, this, &CameraClient::onDisconnected);
    connect(&socket_, &QTcpSocket::readyRead, this, &CameraClient::onReadyRead);
    connect(&socket_, &QTcpSocket::errorOccurred, this, &CameraClient::onErrorOccurred);
}

void CameraClient::startup()
{

    const wchar_t* exeName = L"MMCameraCtrl.exe";
    const wchar_t* exePath = L"D:\\MagicMirror\\git\\qt_test\\MMCameraCtrl\\Debug\\MMCameraCtrl.exe";

    // 1) 是否已启动
    if (!isProcessRunning(exeName)) {
        emit log("MMCameraCtrl not running, starting...");
        if (!startProcess(exePath)) {
            emit fatalError("Failed to start MMCameraCtrl.exe");
            return;
        }
    } else {
        emit log("MMCameraCtrl already running.");
    }

    // 2) 等待服务就绪
    if (!waitForServerReady("127.0.0.1", 52345, 5000)) {
        emit fatalError("MMCameraCtrl did not become ready in time.");
        return;
    }


    //连接 server

    connectToServer("127.0.0.1", 52345);
}

void CameraClient::connectToServer(const QString& host, quint16 port) {
    if (socket_.state() != QAbstractSocket::UnconnectedState)
        socket_.abort();
    emit log(QString("Connecting to %1:%2 ...").arg(host).arg(port));
    socket_.connectToHost(host, port);
}

void CameraClient::disconnectFromServer() {
    previewOn_ = false;
    emit previewOnChanged();
    socket_.disconnectFromHost();
}

void CameraClient::openCamera() {
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "open";
    obj["series"] = CanonEOS;

    sendCommandRequest(obj);
}

void CameraClient::startPreview() {
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "startpreview";

    sendCommandRequest(obj);

    previewOn_ = true;
    emit previewOnChanged();
}

void CameraClient::capture() {
    if (!connected()) { emit log("Not connected."); return; }
    if (!previewOn()) { emit log("Not previewOn."); return; }
    QJsonObject obj;
    obj["cmd"] = "capture";

    sendCommandRequest(obj);

    emit previewOnChanged();
}

void CameraClient::stopPreview()
{
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "stoppreview";

    sendCommandRequest(obj);

    previewOn_ = true;
    emit previewOnChanged();
}


void CameraClient::closeCamera() {
    if (!connected()) { emit log("Not connected."); return; }
    QJsonObject obj;
    obj["cmd"] = "exit";

    sendCommandRequest(obj);
    previewOn_ = false;

    // 2) 尝试把数据真正送出去（给一点时间）
    socket_.flush();
    socket_.waitForBytesWritten(200);

    // 3) 正常断开
    socket_.disconnectFromHost();
    socket_.waitForDisconnected(200);
    //emit previewOnChanged();
}

bool CameraClient::sendMessage(quint16 msgType, quint32 requestId, const QByteArray& payload)
{
    if (socket_.state() != QAbstractSocket::ConnectedState)
        return false;

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

    // 4) 一次性写入
    qint64 n = socket_.write(buf);
    if (n != buf.size()) {
        // write 可能是异步写入到 Qt 缓冲区，这里 n 通常等于 buf.size()
        // 如果你担心，可加 waitForBytesWritten
    }
    return true;
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
    pending_[reqId] = [this](const QJsonObject& resp){
        QString cmd = resp.value("cmd").toString();
        QString result = resp.value("result").toString();
        emit log(QString("RESP  cmd=%1 result=%2 ")
                 .arg(cmd)
                 .arg(result));
    };
    return reqId;
}

void CameraClient::onConnected() {
    emit log("Connected.");
    emit connectedChanged();
    // 程序启动连接成功后，自动 open camera
    openCamera();
}

void CameraClient::onDisconnected() {
    emit log("Disconnected.");
    emit connectedChanged();
    previewOn_ = false;
    emit previewOnChanged();
    rxBuf_.clear();
}

void CameraClient::onErrorOccurred(QAbstractSocket::SocketError) {
    emit log(QString("Socket error: %1").arg(socket_.errorString()));
    emit connectedChanged();
}

void CameraClient::onReadyRead()
{
    rxBuf_ += socket_.readAll();

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
            socket_.disconnectFromHost();
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
            handleFrameData(reqId, payload);
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

void CameraClient::handleFrameData(quint32 reqId, const QByteArray& payload)
{

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

bool CameraClient::tryParseOneMessage() {
    if (rxBuf_.size() < HEADER_LEN) return false;

    // TODO: 按你真实 header 解包
    QDataStream ds(rxBuf_);
    ds.setByteOrder(QDataStream::LittleEndian);

    quint32 magic = 0;
    quint16 msgType = 0;
    quint16 reserved = 0;
    quint32 bodyLen = 0;
    quint32 reserved2 = 0;

    ds >> magic >> msgType >> reserved >> bodyLen >> reserved2;

    if (magic != 0x43414D30) {
        emit log("Bad magic. Drop buffer.");
        rxBuf_.clear();
        return false;
    }

    const int totalLen = HEADER_LEN + (int)bodyLen;
    if (rxBuf_.size() < totalLen) return false; // 半包

    QByteArray body = rxBuf_.mid(HEADER_LEN, bodyLen);
    rxBuf_.remove(0, totalLen);

    // 处理消息
    if (msgType == MSG_FRAME_PREVIEW) {
        QImage img;
        if (img.loadFromData(body)) {
            onFrameImageDecoded(img);
        } else {
            emit log("Failed to decode image frame.");
        }
    } else {
        // 其他消息：ACK / 状态 / 错误码
        emit log(QString("Recv msgType=0x%1 bodyLen=%2").arg(msgType, 0, 16).arg(body.size()));
    }

    return true;
}
