#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>

class BackupManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool indeterminatePhase READ indeterminatePhase NOTIFY indeterminatePhaseChanged)
public:
    explicit BackupManager(QObject *parent = nullptr);

    bool indeterminatePhase() const { return m_indeterminatePhase; }

    // Q_INVOKABLE 使该方法可以从 QML 调用
    Q_INVOKABLE void startBackup(const QString &savePath, bool includePhotos);
    Q_INVOKABLE void startRestore(const QString &zipPath);

signals:
    void backupProgress(double progress); // 0.0 到 1.0
    void restoreProgress(double progress);
    void finished(bool success, const QString &message);
    void indeterminatePhaseChanged();

private:
    void setIndeterminatePhase(bool v);
    void startProgressTick(double baseProgress, double maxProgress, double stepPerSec);
    void stopProgressTick();
    void onZipProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onRestoreExpandDone(); // 解压完成后继续恢复的后续步骤

    bool m_indeterminatePhase = false;
    QProcess *m_process = nullptr;
    QTimer *m_progressTimer = nullptr;
    int m_progressTickCount = 0;
    double m_progressBase = 0;
    double m_progressMax = 0;
    double m_progressStep = 0.04;
    bool m_isRestorePhase = false;
    QString m_tempDir;
    QString m_localPath;
    QString m_restoreAppDir;
    QString m_restoreTempExtractPath;
    QString m_restoreTempOldPath;
};

#endif // BACKUPMANAGER_H
