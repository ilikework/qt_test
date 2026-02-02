#include "backupmanager.h"
#include <private/qzipwriter_p.h>
#include <private/qzipreader_p.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDirIterator>
#include "AppDb.h"
#include "MM_Const_Define.h"

BackupManager::BackupManager(QObject *parent) : QObject(parent) {}

void BackupManager::startBackup(const QString &savePath, bool includePhotos) {
    // 1. 处理 QML 传过来的路径 (去掉 "file:///")
    QString localPath = QUrl(savePath).toLocalFile();
    if(localPath.isEmpty()) localPath = savePath;

    QZipWriter zip(localPath);
    if (zip.status() != QZipWriter::NoError) {
        emit finished(false, "无法创建备份文件");
        return;
    }

    // 设置数据库路径和图片目录 (根据你实际项目配置)
    // 1. 获取 exe 所在的绝对路径
    QString appDir = QCoreApplication::applicationDirPath();

    // 2. 拼接数据库的绝对路径
    // 使用 QDir::cleanPath 确保斜杠方向在不同系统下都正确
    QString dbPath = QDir::cleanPath(appDir + "/" + DB_FILENAME);


    // 3. 递归备份 db 和 customers 文件夹
    QString customersDir = QDir::cleanPath(appDir + SLASH + DIR_CUSTOMERS);
    // --- 進捗計算の準備 ---
    QStringList fileList;
    if (QFile::exists(dbPath)) fileList << dbPath;

    if (includePhotos && QDir(customersDir).exists()) {
        QDirIterator it(customersDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            fileList << it.next();
        }
    }
    int totalFiles = fileList.count();
    if (totalFiles == 0) {
        emit finished(false, "Cannot found backup file.");
        return;
    }

    // --- backup実行 ---
    for (int i = 0; i < totalFiles; ++i) {
        QString currentFilePath = fileList.at(i);
        QFileInfo fileInfo(currentFilePath);

        // ZIP内の相対パスを決定
        QString relativeInZip;
        if (currentFilePath == dbPath) {
            relativeInZip = DB_FILENAME;
            zip.setCompressionPolicy(QZipWriter::AlwaysCompress);
        } else {
            relativeInZip = "customers/" + QDir(customersDir).relativeFilePath(currentFilePath);
            // JPGは圧縮しない、それ以外(obj, mtl, db)は圧縮する
            QString ext = fileInfo.suffix().toLower();
            if (ext == "jpg" || ext == "jpeg") {
                zip.setCompressionPolicy(QZipWriter::NeverCompress);
            } else {
                zip.setCompressionPolicy(QZipWriter::AlwaysCompress);
            }
        }

        QFile file(currentFilePath);
        if (file.open(QIODevice::ReadOnly)) {
            zip.addFile(relativeInZip, file.readAll());
            file.close();
        }

        // 進捗率をQMLへ送信 (0.0 ～ 1.0)
        emit backupProgress(static_cast<double>(i + 1) / totalFiles);
    }

    zip.close();
    emit finished(true, "备份成功！");
}

void BackupManager::startRestore(const QString &zipPath) {
    QString appDir = QCoreApplication::applicationDirPath();
    QString tempExtractPath = QDir::cleanPath(appDir + "/temp_extract");
    QString tempOldPath = QDir::cleanPath(appDir + "/temp_old_backup");

    QString localZip = QUrl(zipPath).toLocalFile();
    if(localZip.isEmpty()) localZip = zipPath;

    // 清理残留的临时目录
    auto cleanTemp = [](QString path) { QDir(path).removeRecursively(); };
    cleanTemp(tempExtractPath);
    cleanTemp(tempOldPath);

    // --- 阶段 1: 解压并校验 (0% - 40%) ---
    QZipReader zip(localZip);
    if (zip.status() != QZipReader::NoError) {
        emit finished(false, "备份文件损坏");
        return;
    }

    auto files = zip.fileInfoList();
    for (int i = 0; i < files.count(); ++i) {
        QString target = QDir::cleanPath(tempExtractPath + "/" + files[i].filePath);
        if (files[i].isDir) QDir().mkpath(target);
        else {
            QDir().mkpath(QFileInfo(target).path());
            QFile f(target);
            if (f.open(QIODevice::WriteOnly)) { f.write(zip.fileData(files[i].filePath)); f.close(); }
        }
        emit backupProgress(0.4 * (i + 1) / files.count());
    }
    zip.close();

    // 校验：必须包含数据库
    if (!QFile::exists(tempExtractPath + "/MMFace_.db")) {
        cleanTemp(tempExtractPath);
        emit finished(false, "非法备份包：缺少数据库文件");
        return;
    }

    // --- 阶段 2: 转移旧数据 (40% - 60%) ---
    emit backupProgress(0.5);
    AppDb::instance().closeDb(); // 必须关闭

    QDir().mkpath(tempOldPath);
    auto moveItem = [](const QString& src, const QString& dst) -> bool {
        if (!QFile::exists(src)) return true;
        return QFile::rename(src, dst);
    };

    // 暂存旧数据
    bool backupOldSuccess = true;
    backupOldSuccess &= moveItem(appDir + SLASH + DB_FILENAME, tempOldPath + SLASH + DB_FILENAME);
    backupOldSuccess &= moveItem(appDir + SLASH + DIR_CUSTOMERS, tempOldPath + SLASH + DIR_CUSTOMERS);
    // 同时也移动 WAL 模式的临时文件（如果有）
    moveItem(appDir + "/MMFace_.db-shm", tempOldPath + "/MMFace_.db-shm");
    moveItem(appDir + "/MMFace_.db-wal", tempOldPath + "/MMFace_.db-wal");

    if (!backupOldSuccess) {
        // 如果连移走旧数据都失败（权限问题），直接恢复连接并退出
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
        emit backupProgress(0.6 + 0.3 * (i + 1) / items.count());
    }

    // --- 阶段 4: 结果处理与回滚 (90% - 100%) ---
    if (deploySuccess) {
        cleanTemp(tempExtractPath);
        cleanTemp(tempOldPath); // 成功了，删除旧备份
        AppDb::instance().openDb();
        emit backupProgress(1.0);
        emit finished(true, "恢复成功！");
    } else {
        // 【回滚逻辑】
        // 1. 先清理生产目录中“部署了一半”的文件/文件夹，确保回归空白状态
        // 否则下面的 rename(old, app) 会因为目标已存在而失败
        for (const QString& item : items) {
            QString deployedItem = appDir + "/" + item;
            if (QFileInfo(deployedItem).isDir()) {
                QDir(deployedItem).removeRecursively();
            } else {
                QFile::remove(deployedItem);
            }
        }

        // 2. 将暂存的旧数据移回原位
        moveItem(tempOldPath + "/MMFace_.db", appDir + "/MMFace_.db");
        moveItem(tempOldPath + "/customers", appDir + "/customers");
        moveItem(tempOldPath + "/MMFace_.db-shm", appDir + "/MMFace_.db-shm");
        moveItem(tempOldPath + "/MMFace_.db-wal", appDir + "/MMFace_.db-wal");

        // 3. 重启数据库并清理所有临时目录
        AppDb::instance().openDb();
        cleanTemp(tempExtractPath);
        cleanTemp(tempOldPath);
        emit finished(false, "部署失败，已自动回滚至原始数据。");
    }
}
