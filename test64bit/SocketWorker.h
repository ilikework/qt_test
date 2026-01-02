#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>


class SocketWorker : public QObject {
    Q_OBJECT
public:
    explicit SocketWorker(QObject* parent=nullptr) : QObject(parent) {}

public slots:
    void startup(const QString& host, quint16 port);
    void send(const QByteArray& data);

    void shutdown();

signals:
    // 把事件“转发”回 UI
    void connected();
    void disconnected();
    void readyReadData(QByteArray chunk);
    void errorOccurredStr(QString err);


    void fatalError(const QString& msg);
    void log(const QString& msg);
    void exitFinished(bool ok);

private slots:
    void onConnected() { emit connected(); }
    void onDisconnected() { emit disconnected(); }
    void beginExitWait(quint32 reqId, int timeoutMs);
    void notifyExitResp(quint32 reqId);

    void onReadyRead() {
        emit readyReadData(socket_->readAll());
    }

    void onErrorOccurred(QAbstractSocket::SocketError) {
        emit errorOccurredStr(socket_->errorString());
    }

private:
    void connectToServer(const QString& host, quint16 port);


    QTcpSocket* socket_ = nullptr;
    QMutex exitMtx_;
    QWaitCondition exitCv_;
    bool exitWaiting_ = false;
    bool exitOk_ = false;
    quint32 exitReqId_ = 0;
};

