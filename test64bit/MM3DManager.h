#ifndef MM3DMANAGER_H
#define MM3DMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>

/**
 * 通过管道调用 FaceReconCPU.exe 生成 3D 人脸 obj 与贴图。
 * 协议：子进程输出 "init" 即表示就绪；主进程直接发送 "xxxx.json\r\n"（JSON 文件绝对路径）；
 * 再读一行，'1' 表示成功；最后发送 "exit\r\n"。
 * JSON 由 QML 构造（如 8 对图：左/右贴图数组），格式：{"model": N, "info": [左图obj, 左图贴图数组, 右图obj, 右图贴图数组, 输出目录]}。
 */
class MM3DManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString exePath READ exePath WRITE setExePath NOTIFY exePathChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit MM3DManager(QObject *parent = nullptr);

    QString exePath() const { return exePath_; }
    void setExePath(const QString &path);
    bool running() const { return running_; }

    /// jsonContent 为 QML 构造的完整 JSON 字符串（含 model、info）；C++ 仅写入文件并调 exe。
    Q_INVOKABLE void runFaceRecon(const QString &jsonContent);

    Q_INVOKABLE bool fileExists(const QString &localPath) const;

signals:
    void exePathChanged();
    void runningChanged();
    /// success: 是否生成成功；outputDir: 输出目录（可据此拼接 obj/贴图路径）
    void finished(bool success, const QString &outputDir);
    void errorMessage(const QString &msg);

private:
    void setRunning(bool on);
    void sendCommand(const QString &cmd);
    void onReadyReadStandardOutput();
    void onProcessFinished(int code, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError err);

    QString exePath_;
    QProcess process_;
    QByteArray stdoutBuffer_;
    QString pendingCommand_;
    enum State { Idle, WaitingInit, WaitingResult } state_ = Idle;
    QString lastOutputDir_;
    bool running_ = false;
};

#endif // MM3DMANAGER_H
