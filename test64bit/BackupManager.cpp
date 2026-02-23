#include "BackupManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <QDirIterator>
#include "AppDb.h"
#include "MM_Const_Define.h"

BackupManager::BackupManager(QObject *parent) : QObject(parent) {}

static QString toNativePath(const QString &path) {
    return QDir::toNativeSeparators(path);
}

void BackupManager::startBackup(const QString &savePath, bool includePhotos) {
    QString localPath = QUrl(savePath).toLocalFile();
    if (localPath.isEmpty()) localPath = savePath;

    QString appDir = QCoreApplication::applicationDirPath();
    QString dbPath = QDir::cleanPath(appDir + "/" + DB_FILENAME);
    QString customersDir = QDir::cleanPath(appDir + SLASH + DIR_CUSTOMERS);

    if (!QFile::exists(dbPath)) {
        emit finished(false, "Cannot found backup file.");
        return;
    }

    // 使用 PowerShell Compress-Archive：先复制到临时目录再压缩，以控制 zip 内路径
    QString tempDir = QDir::tempPath() + "/mm_backup_" + QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(tempDir);

    auto cleanTemp = [](const QString &path) { QDir(path).removeRecursively(); };

    // 复制 db
    if (!QFile::copy(dbPath, tempDir + "/" + DB_FILENAME)) {
        cleanTemp(tempDir);
        emit finished(false, "无法复制数据库文件");
        return;
    }

    if (includePhotos && QDir(customersDir).exists()) {
        QString destCust = tempDir + "/" + DIR_CUSTOMERS;
        QDir().mkpath(destCust);
        QDirIterator it(customersDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        QStringList allFiles;
        while (it.hasNext()) allFiles << it.next();
        int n = 0;
        for (const QString &src : allFiles) {
            QString rel = QDir(customersDir).relativeFilePath(src);
            QString dst = destCust + "/" + rel;
            QDir().mkpath(QFileInfo(dst).path());
            QFile::copy(src, dst);
            emit backupProgress(0.4 * (++n) / qMax(1, allFiles.size()));
        }
    }

    emit backupProgress(0.5);

    // PowerShell: Compress-Archive -Path temp\* -DestinationPath out.zip -Force
    QString psScript = QString(
        "Set-Location -LiteralPath '%1'; "
        "Compress-Archive -Path '*' -DestinationPath '%2' -Force"
    ).arg(toNativePath(tempDir), toNativePath(localPath));

    int exitCode = QProcess::execute("powershell.exe", QStringList()
        << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);

    cleanTemp(tempDir);

    if (exitCode != 0) {
        emit finished(false, "压缩备份失败");
        return;
    }
    emit backupProgress(1.0);
    emit finished(true, "备份成功！");
}

void BackupManager::startRestore(const QString &zipPath) {
    QString appDir = QCoreApplication::applicationDirPath();
    QString tempExtractPath = QDir::cleanPath(appDir + "/temp_extract");
    QString tempOldPath = QDir::cleanPath(appDir + "/temp_old_backup");

    QString localZip = QUrl(zipPath).toLocalFile();
    if (localZip.isEmpty()) localZip = zipPath;

    auto cleanTemp = [](const QString &path) { QDir(path).removeRecursively(); };
    cleanTemp(tempExtractPath);
    cleanTemp(tempOldPath);
    QDir().mkpath(tempExtractPath);

    // PowerShell: Expand-Archive -Path zip -DestinationPath temp -Force
    QString psScript = QString(
        "Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force"
    ).arg(toNativePath(localZip), toNativePath(tempExtractPath));

    int exitCode = QProcess::execute("powershell.exe", QStringList()
        << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);

    if (exitCode != 0) {
        cleanTemp(tempExtractPath);
        emit finished(false, "备份文件损坏或解压失败");
        return;
    }

    emit backupProgress(0.4);

    // 校验：必须包含数据库
    if (!QFile::exists(tempExtractPath + "/MMFace_.db")) {
        cleanTemp(tempExtractPath);
        emit finished(false, "非法备份包：缺少数据库文件");
        return;
    }

    // --- 阶段 2: 转移旧数据 (40% - 60%) ---
    emit backupProgress(0.5);
    AppDb::instance().closeDb();

    QDir().mkpath(tempOldPath);
    auto moveItem = [](const QString &src, const QString &dst) -> bool {
        if (!QFile::exists(src)) return true;
        return QFile::rename(src, dst);
    };

    bool backupOldSuccess = true;
    backupOldSuccess &= moveItem(appDir + SLASH + DB_FILENAME, tempOldPath + SLASH + DB_FILENAME);
    backupOldSuccess &= moveItem(appDir + SLASH + DIR_CUSTOMERS, tempOldPath + SLASH + DIR_CUSTOMERS);
    moveItem(appDir + "/MMFace_.db-shm", tempOldPath + "/MMFace_.db-shm");
    moveItem(appDir + "/MMFace_.db-wal", tempOldPath + "/MMFace_.db-wal");

    if (!backupOldSuccess) {
        AppDb::instance().openDb();
        cleanTemp(tempExtractPath);
        emit finished(false, "无法暂存旧数据，恢复终止");
        return;
    }

    // --- 阶段 3: 部署新数据 (60% - 90%) ---
    QDir extractDir(tempExtractPath);
    QStringList items = extractDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    bool deploySuccess = true;

    for (int i = 0; i < items.count(); ++i) {
        if (!moveItem(tempExtractPath + "/" + items[i], appDir + "/" + items[i])) {
            deploySuccess = false;
            break;
        }
        emit backupProgress(0.6 + 0.3 * (i + 1) / qMax(1, items.count()));
    }

    // --- 阶段 4: 结果处理与回滚 (90% - 100%) ---
    if (deploySuccess) {
        cleanTemp(tempExtractPath);
        cleanTemp(tempOldPath);
        AppDb::instance().openDb();
        emit backupProgress(1.0);
        emit finished(true, "恢复成功！");
    } else {
        for (const QString &item : items) {
            QString deployedItem = appDir + "/" + item;
            if (QFileInfo(deployedItem).isDir()) {
                QDir(deployedItem).removeRecursively();
            } else {
                QFile::remove(deployedItem);
            }
        }
        moveItem(tempOldPath + "/MMFace_.db", appDir + "/MMFace_.db");
        moveItem(tempOldPath + "/customers", appDir + "/customers");
        moveItem(tempOldPath + "/MMFace_.db-shm", appDir + "/MMFace_.db-shm");
        moveItem(tempOldPath + "/MMFace_.db-wal", appDir + "/MMFace_.db-wal");
        AppDb::instance().openDb();
        cleanTemp(tempExtractPath);
        cleanTemp(tempOldPath);
        emit finished(false, "部署失败，已自动回滚至原始数据。");
    }
}
