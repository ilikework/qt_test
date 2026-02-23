#include "MM3DManager.h"
#include "appconfig.h"
#include <QFileInfo>
#include <QDir>

MM3DManager::MM3DManager(QObject *parent)
    : QObject{parent}
{
    exePath_ = AppConfig::instance().FaceReconExePath();

    connect(&process_, &QProcess::readyReadStandardOutput, this, &MM3DManager::onReadyReadStandardOutput);
    connect(&process_, &QProcess::finished, this, &MM3DManager::onProcessFinished);
    connect(&process_, &QProcess::errorOccurred, this, &MM3DManager::onProcessError);
}

void MM3DManager::setExePath(const QString &path)
{
    if (exePath_ == path) return;
    exePath_ = path;
    emit exePathChanged();
}

void MM3DManager::setRunning(bool on)
{
    if (running_ == on) return;
    running_ = on;
    emit runningChanged();
}

bool MM3DManager::fileExists(const QString &localPath) const
{
    return QFileInfo::exists(localPath);
}

void MM3DManager::runFaceRecon(const QString &fileIn1, const QString &fileTexture1,
                               const QString &fileIn2, const QString &fileTexture2,
                               const QString &outputDir, int type)
{
    if (running_) {
        emit errorMessage("FaceRecon is already running.");
        return;
    }

    QFileInfo fi(exePath_);
    if (!fi.exists()) {
        emit errorMessage("FaceReconCPU.exe not found: " + exePath_);
        emit finished(false, QString());
        return;
    }

    QString workDir = fi.absolutePath();
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        if (!outDir.mkpath(".")) {
            emit errorMessage("Cannot create output dir: " + outputDir);
            emit finished(false, QString());
            return;
        }
    }

    // 与 VC 一致：type + 空格 + 5 个参数 + \r\n。exe 按输入图命名产出，如 01_L-01_R.obj、01_L-01_R_texture.jpg
    QString cmd = QString::number(type) + " " + fileIn1 + " " + fileTexture1 + " "
                  + fileIn2 + " " + fileTexture2 + " " + outputDir + "\r\n";

    stdoutBuffer_.clear();
    pendingCommand_ = cmd;
    lastOutputDir_ = outputDir;
    state_ = WaitingInit;
    setRunning(true);

    process_.setWorkingDirectory(workDir);
    process_.start(exePath_, QStringList(), QProcess::ReadWrite);
    if (!process_.waitForStarted(5000)) {
        setRunning(false);
        emit errorMessage("Failed to start: " + exePath_);
        emit finished(false, QString());
        return;
    }
}

void MM3DManager::sendCommand(const QString &cmd)
{
    process_.write(cmd.toUtf8());
}

void MM3DManager::onReadyReadStandardOutput()
{
    stdoutBuffer_.append(process_.readAllStandardOutput());

    if (state_ == WaitingInit) {
        if (stdoutBuffer_.contains("init\r\n") || stdoutBuffer_.contains("init\n")) {
            state_ = WaitingResult;
            sendCommand(pendingCommand_);
        }
    }

    if (state_ == WaitingResult) {
        int idx = stdoutBuffer_.indexOf('\n');
        if (idx >= 0) {
            QByteArray line = stdoutBuffer_.left(idx).trimmed();
            if (!line.isEmpty()) {
                bool success = (line.at(0) == '1');
                state_ = Idle;
                setRunning(false);
                sendCommand("exit\r\n");
                process_.closeWriteChannel();
                emit finished(success, lastOutputDir_);
            }
        }
    }
}

void MM3DManager::onProcessFinished(int code, QProcess::ExitStatus status)
{
    if (state_ != Idle && running_) {
        setRunning(false);
        state_ = Idle;
        emit errorMessage("Process exited before result: code=" + QString::number(code));
        emit finished(false, QString());
    }
}

void MM3DManager::onProcessError(QProcess::ProcessError err)
{
    if (running_) {
        setRunning(false);
        state_ = Idle;
        QString msg = "Process error: " + QString::number(static_cast<int>(err));
        if (err == QProcess::FailedToStart)
            msg = "Failed to start " + exePath_;
        emit errorMessage(msg);
        emit finished(false, QString());
    }
}
