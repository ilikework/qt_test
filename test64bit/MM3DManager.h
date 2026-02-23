#ifndef MM3DMANAGER_H
#define MM3DMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>

/**
 * 通过管道调用 FaceReconCPU.exe 生成 3D 人脸 obj 与贴图。
 * 协议：子进程先输出 "init\r\n"，主进程发送 "type fileIn1 fileTexture1 fileIn2 fileTexture2 outputPath\r\n"，
 * 再读一行，首字符 '1' 表示成功；最后发送 "exit\r\n"。
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

    /// 参数为本地文件路径。outputDir 为 exe 输出目录；exe 按输入图命名产出，如 01_L.jpg+01_R.jpg → 01_L-01_R.obj、01_L-01_R_texture.jpg。
    Q_INVOKABLE void runFaceRecon(const QString &fileIn1, const QString &fileTexture1,
                                   const QString &fileIn2, const QString &fileTexture2,
                                   const QString &outputDir, int type);

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
