#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QObject>

class BackupManager : public QObject
{
    Q_OBJECT
public:
    explicit BackupManager(QObject *parent = nullptr);

    // Q_INVOKABLE 使该方法可以从 QML 调用
    Q_INVOKABLE void startBackup(const QString &savePath, bool includePhotos);
    Q_INVOKABLE void startRestore(const QString &zipPath);

signals:
    void backupProgress(double progress); // 0.0 到 1.0
    void restoreProgress(double progress);
    void finished(bool success, const QString &message);
};

#endif // BACKUPMANAGER_H
