#include "MM3DManager.h"
#include "appconfig.h"
#include "MMLogger.h"
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
        LOG(QString("[MM3DManager] FaceReconCPU.exe not found: %1").arg(exePath_));
        emit errorMessage("FaceReconCPU.exe not found: " + exePath_);
        emit finished(false, QString());
        return;
    }

    QString workDir = fi.absolutePath();
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        if (!outDir.mkpath(".")) {
            LOG(QString("[MM3DManager] Cannot create output dir: %1").arg(outputDir));
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
    LOG(QString("[MM3DManager] start exe, workDir: %1 cmd: %2").arg(workDir, cmd.trimmed()));
    process_.start(exePath_, QStringList(), QProcess::ReadWrite);
    if (!process_.waitForStarted(5000)) {
        setRunning(false);
        LOG(QString("[MM3DManager] Failed to start exe: %1").arg(exePath_));
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
    LOG(QString("[MM3DManager] stdout raw: %1").arg(QString::fromUtf8(stdoutBuffer_)));

    if (state_ == WaitingInit) {
        if (stdoutBuffer_.contains("init\r\n") || stdoutBuffer_.contains("init\n")) {
            LOG("[MM3DManager] got init, send command");
            state_ = WaitingResult;
            sendCommand(pendingCommand_);
            // 丢掉 "init\r\n" 这一行，否则下面会误把 init 当结果行解析
            int firstNewline = stdoutBuffer_.indexOf('\n');
            if (firstNewline >= 0)
                stdoutBuffer_.remove(0, firstNewline + 1);
        }
    }

    if (state_ == WaitingResult) {
        int idx = stdoutBuffer_.indexOf('\n');
        if (idx >= 0) {
            QByteArray line = stdoutBuffer_.left(idx).trimmed();
            int firstByte = line.isEmpty() ? -1 : int(static_cast<unsigned char>(line.at(0)));
            LOG(QString("[MM3DManager] result line: %1 firstByte: %2").arg(QString::fromUtf8(line)).arg(firstByte));
            if (!line.isEmpty()) {
                bool success = (line.at(0) == '1');
                LOG(QString("[MM3DManager] parsed success: %1 outputDir: %2").arg(success ? 1 : 0).arg(lastOutputDir_));
                state_ = Idle;
                setRunning(false);
                sendCommand("exit\r\n");
                process_.closeWriteChannel();
                if (!success)
                    emit errorMessage(QStringLiteral("3D生成失败"));
                emit finished(success, lastOutputDir_);
            }
        }
    }
}

void MM3DManager::onProcessFinished(int code, QProcess::ExitStatus status)
{
    LOG(QString("[MM3DManager] onProcessFinished code: %1 status: %2 state_: %3 running_: %4")
        .arg(code).arg(int(status)).arg(state_).arg(running_ ? 1 : 0));
    if (state_ != Idle && running_) {
        setRunning(false);
        state_ = Idle;
        LOG(QString("[MM3DManager] Process exited before result: code=%1").arg(code));
        emit errorMessage("Process exited before result: code=" + QString::number(code));
        emit finished(false, QString());
    }
}

void MM3DManager::onProcessError(QProcess::ProcessError err)
{
    LOG(QString("[MM3DManager] onProcessError: %1 (exe: %2)").arg(int(err)).arg(exePath_));
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
