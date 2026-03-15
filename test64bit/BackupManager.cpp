#include "BackupManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <QDirIterator>
#include "AppDb.h"
#include "MM_Const_Define.h"
#include "MMLogger.h"

BackupManager::BackupManager(QObject *parent) : QObject(parent) {
    m_process = new QProcess(this);
    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(1000);
    connect(m_progressTimer, &QTimer::timeout, this, [this]() {
        if (!m_progressTimer->isActive()) return;
        ++m_progressTickCount;
        double p = m_progressBase + m_progressStep * m_progressTickCount;
        if (p > m_progressMax) p = m_progressMax;
        emit backupProgress(p);
        if (m_isRestorePhase) emit restoreProgress(p);
    });
    connect(m_process, &QProcess::finished, this, [this]() {
        onZipProcessFinished(m_process->exitCode(), m_process->exitStatus());
    });
}

void BackupManager::setIndeterminatePhase(bool v) {
    if (m_indeterminatePhase != v) {
        m_indeterminatePhase = v;
        emit indeterminatePhaseChanged();
    }
}

void BackupManager::startProgressTick(double baseProgress, double maxProgress, double stepPerSec) {
    m_progressBase = baseProgress;
    m_progressMax = maxProgress;
    m_progressStep = stepPerSec;
    m_progressTickCount = 0;
    emit backupProgress(baseProgress);
    if (m_isRestorePhase) emit restoreProgress(baseProgress);
    m_progressTimer->start();
}

void BackupManager::stopProgressTick() {
    m_progressTimer->stop();
}

void BackupManager::onZipProcessFinished(int exitCode, QProcess::ExitStatus /*status*/) {
    stopProgressTick();
    setIndeterminatePhase(false);
    if (m_isRestorePhase) {
        if (exitCode != 0) {
            QDir(m_restoreTempExtractPath).removeRecursively();
            emit finished(false, "备份文件损坏或解压失败");
            return;
        }
        onRestoreExpandDone();
        return;
    }
    auto cleanTemp = [](const QString &path) { QDir(path).removeRecursively(); };
    cleanTemp(m_tempDir);
    if (exitCode != 0) {
        emit finished(false, "压缩备份失败");
        return;
    }
    emit backupProgress(1.0);
    emit finished(true, "备份成功！");
}

void BackupManager::onRestoreExpandDone() {
    auto cleanTemp = [](const QString &path) { QDir(path).removeRecursively(); };
    QString appDir = m_restoreAppDir;
    QString tempExtractPath = m_restoreTempExtractPath;
    QString tempOldPath = m_restoreTempOldPath;

    emit backupProgress(0.4);
    emit restoreProgress(0.4);
    if (!QFile::exists(tempExtractPath + "/MMFace_.db")) {
        cleanTemp(tempExtractPath);
        emit finished(false, "非法备份包：缺少数据库文件");
        return;
    }
    emit backupProgress(0.5);
    emit restoreProgress(0.5);
    AppDb::instance().closeDb();

    QDir().mkpath(tempOldPath);
    auto moveItem = [](const QString &src, const QString &dst) -> bool {
        if (!QFile::exists(src)) return true;
        return QFile::rename(src, dst);
    };

    const QString dbSrc = appDir + SLASH + DB_FILENAME;
    const QString dbDst = tempOldPath + SLASH + DB_FILENAME;
    const QString custSrc = appDir + SLASH + DIR_CUSTOMERS;
    const QString custDst = tempOldPath + SLASH + DIR_CUSTOMERS;

    bool movedDb = moveItem(dbSrc, dbDst);
    bool movedCust = moveItem(custSrc, custDst);
    moveItem(appDir + "/MMFace_.db-shm", tempOldPath + "/MMFace_.db-shm");
    moveItem(appDir + "/MMFace_.db-wal", tempOldPath + "/MMFace_.db-wal");

    if (!movedDb || !movedCust) {
        if (!movedDb)
            LOG(QString("[BackupManager] 暂存失败: 无法移动数据库 %1 -> %2").arg(dbSrc, dbDst));
        if (!movedCust)
            LOG(QString("[BackupManager] 暂存失败: 无法移动目录 %1 -> %2").arg(custSrc, custDst));
        AppDb::instance().openDb();
        cleanTemp(tempExtractPath);
        QString detail = !movedDb && !movedCust ? "（数据库与顾客目录可能被占用）"
            : !movedDb ? "（数据库文件被占用或权限不足）"
            : "（顾客目录被占用或权限不足，请关闭可能占用该目录的程序）";
        emit finished(false, "无法暂存旧数据" + detail + "，恢复终止");
        return;
    }

    QDir extractDir(tempExtractPath);
    QStringList items = extractDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    bool deploySuccess = true;

    for (int i = 0; i < items.count(); ++i) {
        if (!moveItem(tempExtractPath + "/" + items[i], appDir + "/" + items[i])) {
            deploySuccess = false;
            break;
        }
        double p = 0.6 + 0.3 * (i + 1) / qMax(1, items.count());
        emit backupProgress(p);
        emit restoreProgress(p);
    }

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

    emit backupProgress(0.0);

    // 使用 PowerShell Compress-Archive：先复制到临时目录再压缩，以控制 zip 内路径。复制约占 0–35%，压缩约占 35–100%
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
            emit backupProgress(0.35 * (++n) / qMax(1, allFiles.size()));
        }
    }

    emit backupProgress(0.35);

    m_isRestorePhase = false;
    m_tempDir = tempDir;
    m_localPath = localPath;
    setIndeterminatePhase(true);
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    QString psScript = QString(
        "Set-Location -LiteralPath '%1'; "
        "Compress-Archive -Path '*' -DestinationPath '%2' -Force"
    ).arg(toNativePath(tempDir), toNativePath(localPath));

    m_process->start("powershell.exe", QStringList()
        << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
    startProgressTick(0.35, 0.98, 0.04);
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

    m_isRestorePhase = true;
    m_restoreAppDir = appDir;
    m_restoreTempExtractPath = tempExtractPath;
    m_restoreTempOldPath = tempOldPath;

    setIndeterminatePhase(true);
    emit backupProgress(0.0);
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    QString psScript = QString(
        "Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force"
    ).arg(toNativePath(localZip), toNativePath(tempExtractPath));

    m_process->start("powershell.exe", QStringList()
        << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
    startProgressTick(0.0, 0.36, 0.04);
}
