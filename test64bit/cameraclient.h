#pragma once
#include <QObject>
#include <QVariantList>
#include <QTcpSocket>
#include <QByteArray>
#include "AppDb.h"
#include <QThread>
#include "SocketWorker.h"

class CameraImageProvider;

enum CameraSeries
{
    NoneType =0,
    CanonGX = 1,
    CanonEOS = 4,            // add for 20140219
    Aigo = 5,     // add for 20110602
    TheSony = 7,           // add 20161111
};

struct MessageHeader {
    quint16 msgType;    // network order
    quint16 version;    // network order
    quint32 requestId;  // network order
};

struct FrameDataHeader {
    quint8  frameType;     // 1 byte, no endian issue
    quint8  reserved1;
    quint16 reserved2;
    quint64 seq;           // network order (htonll)
    quint64 timestampMs;   // network order (htonll)
    quint32 width;         // network order
    quint32 height;        // network order
    quint32 pixelFormat;   // network order
    quint32 dataLength;    // network order
};


class CameraClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(int frameToken READ frameToken NOTIFY frameTokenChanged) // 用来刷新 Image
    Q_PROPERTY(bool previewOn READ previewOn NOTIFY previewOnChanged)
    Q_PROPERTY(QVariantList isos READ isos NOTIFY isosChanged)
    Q_PROPERTY(QVariantList exposuretimes READ exposuretimes NOTIFY exposuretimesChanged)
    Q_PROPERTY(QVariantList apertures READ apertures NOTIFY aperturesChanged)
    Q_PROPERTY(QVariantList wbs READ wbs NOTIFY wbsChanged)
    Q_PROPERTY(QVariantList imageSizes READ imageSizes NOTIFY imageSizesChanged)
    Q_PROPERTY(QVariantList imageQualities READ imageQualities NOTIFY imageQualitiesChanged)
    Q_PROPERTY(QVariantList settings READ settings NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList left_pics READ left_pics NOTIFY left_picsChanged)
    Q_PROPERTY(QVariantList right_pics READ right_pics NOTIFY right_picsChanged)

public:
    explicit CameraClient(CameraImageProvider* provider, QObject* parent = nullptr);
    ~CameraClient();

    bool connected() const { return connected_; }
    int frameToken() const { return frameToken_; }
    bool previewOn() const { return previewOn_; }

    Q_INVOKABLE void init(const QString & strCustomerID);
    Q_INVOKABLE void startup();
    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void startPreview();
    Q_INVOKABLE void stopPreview();
    Q_INVOKABLE void save();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void capture();
    Q_INVOKABLE void closeCamera();
    Q_INVOKABLE quint32 sendCommandRequest(const QJsonObject& obj); // 返回 requestId
    using RespHandler = std::function<void(const QJsonObject&)>;
    Q_INVOKABLE quint32 sendCommandRequest2(const QJsonObject& obj, RespHandler onResp = {}); // 返回 requestId
    Q_INVOKABLE void onSettingChanged(int rowIndex, const QString& key,const QString& displayText,const QVariant& rawValue);

    QVariantList isos() const;
    QVariantList exposuretimes() const;
    QVariantList apertures() const;
    QVariantList wbs() const;
    QVariantList imageSizes() const;
    QVariantList imageQualities() const;
    QVariantList settings() ;
    QVariantList left_pics() const;
    QVariantList right_pics() const;

    void addleft(const QString& str);
    void addright(const QString& str);
    QString get_save_folder() const;

signals:
    void connectedChanged();
    void frameTokenChanged();
    void previewOnChanged();
    void isosChanged();
    void exposuretimesChanged();
    void aperturesChanged();
    void wbsChanged();
    void imageSizesChanged();
    void imageQualitiesChanged();
    void settingsChanged();
    void left_picsChanged();
    void right_picsChanged();

    void fatalError(const QString& msg);
    void log(const QString& msg);


private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead(const QByteArray& chunk);
    void onErrorOccurred(const QString& err);

private:
    void applyCaptureSettingToCamera(const CaptureSetting &s) ;
    bool sendMessage(quint16 msgType, quint32 requestId, const QByteArray& payload);
    void onFrameImageDecoded(const QImage& img);
    void handleCommandResponse(quint32 requestId, const QByteArray& payload);
    void handleEventNotification(const QByteArray& payload);
    void handleFrameData(quint32 reqId, const QByteArray& payload, quint32 bodyLen);
    CaptureParameter get_camera_params(const QString & capType) const;
    QString FindParamText(const QVariantList & list, const int raw) const;

private:
    //QTcpSocket socket_;
    CameraImageProvider* provider_ = nullptr;

    QThread socketThread_;
    SocketWorker* worker_ = nullptr;

    QByteArray rxBuf_;
    bool connected_ = false;
    bool previewOn_ = false;
    //QStringList isos_, exposuretimes_, apertures_, wbs_, imageSizes_, imageQualities_;

    int frameToken_ = 0; // 每次新帧 ++，QML 用它刷新 source
    std::atomic<quint32> nextReqId_{1};
    QHash<quint32, std::function<void(const QJsonObject&)>> pending_;

    std::atomic_bool exitInProgress_{false};
    std::atomic_bool exitDone_{false};
    QMutex exitMtx_;
    QWaitCondition exitCv_;

    QString CustomerID_ = ""; // not valuable 0.
    int GroupID_ = 0;
    QVariantList  left_pics_;
    QVariantList  right_pics_;
};
