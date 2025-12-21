#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>

class CameraImageProvider;

enum CameraSeries
{
    NoneType =0,
    CanonGX = 1,
    CanonEOS = 4,            // add for 20140219
    Aigo = 5,     // add for 20110602
    TheSony = 7,           // add 20161111
};


class CameraClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(int frameToken READ frameToken NOTIFY frameTokenChanged) // 用来刷新 Image
    Q_PROPERTY(bool previewOn READ previewOn NOTIFY previewOnChanged)

public:
    explicit CameraClient(CameraImageProvider* provider, QObject* parent = nullptr);

    bool connected() const { return socket_.state() == QAbstractSocket::ConnectedState; }
    int frameToken() const { return frameToken_; }
    bool previewOn() const { return previewOn_; }

    Q_INVOKABLE void startup();
    Q_INVOKABLE void connectToServer(const QString& host, quint16 port);
    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void startPreview();
    Q_INVOKABLE void stopPreview();
    Q_INVOKABLE void capture();
    Q_INVOKABLE void closeCamera();
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE quint32 sendCommandRequest(const QJsonObject& obj); // 返回 requestId


signals:
    void connectedChanged();
    void frameTokenChanged();
    void previewOnChanged();
    void fatalError(const QString& msg);
    void log(const QString& msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError);

private:
    bool tryParseOneMessage();
    bool sendMessage(quint16 msgType, quint32 requestId, const QByteArray& payload);
    void onFrameImageDecoded(const QImage& img);
    void handleCommandResponse(quint32 requestId, const QByteArray& payload);
    void handleEventNotification(const QByteArray& payload);
    void handleFrameData(quint32 reqId, const QByteArray& payload);

private:
    QTcpSocket socket_;
    QByteArray rxBuf_;
    bool previewOn_ = false;

    CameraImageProvider* provider_ = nullptr;
    int frameToken_ = 0; // 每次新帧 ++，QML 用它刷新 source
    std::atomic<quint32> nextReqId_{1};
    QHash<quint32, std::function<void(const QJsonObject&)>> pending_;
};
