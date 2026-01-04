#include "SocketWorker.h"
#include <QCoreApplication>
#include <QDir>
#include <windows.h>
#include <tlhelp32.h>
#include <QThread>
#include <QTimer>


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

void SocketWorker::startup(const QString& host, quint16 port)
{
    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::connected,    this, &SocketWorker::onConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &SocketWorker::onDisconnected);
    connect(socket_, &QTcpSocket::readyRead,    this, &SocketWorker::onReadyRead);
    connect(socket_, &QTcpSocket::errorOccurred,this, &SocketWorker::onErrorOccurred);
    const wchar_t* exeName = L"MMCameraCtrl.exe";
    std::wstring exePath =QCoreApplication::applicationDirPath().toStdWString();
    exePath += L"/";
    exePath += exeName;

    // 1) 是否已启动
    if (!isProcessRunning(exeName)) {
        emit log("MMCameraCtrl not running, starting...");
        if (!startProcess(exePath.c_str())) {
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

void SocketWorker::connectToServer(const QString& host, quint16 port)
{
    if (socket_->state() != QAbstractSocket::UnconnectedState)
        socket_->abort();
    emit log(QString("Connecting to %1:%2 ...").arg(host).arg(port));
    socket_->connectToHost(host, port);

}


void SocketWorker::send(const QByteArray& data)
{
    if (!socket_ || socket_->state() != QAbstractSocket::ConnectedState)
        return ;

    const qint64 n = socket_->write(data);
}

void SocketWorker::shutdown()
{
    if (!socket_) return;
    socket_->flush();

    socket_->disconnectFromHost();
}

void SocketWorker::beginExitWait(quint32 reqId, int timeoutMs)
{
    QMutexLocker lk(&exitMtx_);
    exitWaiting_ = true;
    exitOk_ = false;
    exitReqId_ = reqId;

    // 超时保护（不阻塞线程）
    QTimer::singleShot(timeoutMs, this, [this] {
        QMutexLocker lk(&exitMtx_);
        if (!exitWaiting_)
            return;

        exitWaiting_ = false;
        exitOk_ = false;

        emit exitFinished(false);   // 超时失败
    });
}

void SocketWorker::notifyExitResp(quint32 reqId)
{
    QMutexLocker lk(&exitMtx_);

    if (!exitWaiting_ || reqId != exitReqId_)
        return;

    exitWaiting_ = false;
    exitOk_ = true;

    emit exitFinished(true);
}

