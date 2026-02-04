#include "PreRecordManager.h"
#include "AppDb.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QSet>

int PreRecordManager::reportTypeByIndex(int index)
{
    return index < 8 ? (index + 1) : 100;
}

int PreRecordManager::reportLevelByTier(int tier)
{
    if (tier == 0) return 30;  // 好
    if (tier == 1) return 20;  // 中
    return 10;                  // 差
}

QString PreRecordManager::copyOfferingsImageToAppDir(const QString &sourcePath, int offeringIx)
{
    QString path = sourcePath;
    if (path.startsWith("file:///"))
        path = path.mid(8);
    else if (path.startsWith("file://"))
        path = path.mid(7);
    path = path.trimmed();
    if (path.isEmpty()) return QString();

    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) return QString();

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString offeringsDir = QDir(appDir).filePath("offerings");
    if (!QDir().mkpath(offeringsDir)) return QString();

    QString ext = fi.suffix();
    if (ext.isEmpty()) ext = "jpg";
    const qint64 msec = QDateTime::currentMSecsSinceEpoch();
    const QString baseName = offeringIx > 0
        ? QString("offering_%1_%2").arg(offeringIx).arg(msec)
        : QString("offering_0_%1").arg(msec);
    const QString fileName = baseName + "." + ext;
    const QString destPath = QDir(offeringsDir).filePath(fileName);

    if (QFile::exists(destPath) && !QFile::remove(destPath)) return QString();
    if (!QFile::copy(path, destPath)) return QString();

    return "offerings/" + fileName;
}

PreRecordManager::PreRecordManager(QObject *parent)
    : QObject(parent)
{
}

QVariantList PreRecordManager::getReportSuggestions() const
{
    QVariantList out;
    AppDb &db = AppDb::instance();
    const char *labels[] = { "毛 孔", "粉 刺", "深层色斑", "浅层色斑", "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告" };
    for (int i = 0; i < 9; ++i) {
        int rt = reportTypeByIndex(i);
        QVariantMap m;
        m["label"] = QString::fromUtf8(labels[i]);
        m["goodMemo"] = db.getReportTemplateMemo(rt, 30);
        m["mediumMemo"] = db.getReportTemplateMemo(rt, 20);
        m["badMemo"] = db.getReportTemplateMemo(rt, 10);
        out.append(m);
    }
    return out;
}

bool PreRecordManager::setReportMemo(int reportIndex, int tier, const QString &text)
{
    if (reportIndex < 0 || reportIndex > 8 || tier < 0 || tier > 2) return false;
    int rt = reportTypeByIndex(reportIndex);
    int rl = reportLevelByTier(tier);
    return AppDb::instance().setReportTemplateMemo(rt, rl, text);
}

QVariantList PreRecordManager::getProducts() const
{
    QVariantList out;
    const QString appDir = QCoreApplication::applicationDirPath();
    QVector<QVariantMap> rows = AppDb::instance().getOfferingsTemplateList();
    for (const QVariantMap &m : rows) {
        QVariantMap row;
        if (m.contains("IX")) row["IX"] = m["IX"];
        row["name"] = m.value("name");
        QString photoPath = m.value("photoPath").toString();
        if (!photoPath.isEmpty() && !QDir::isAbsolutePath(photoPath))
            photoPath = QDir(appDir).filePath(photoPath);
        row["photoPath"] = photoPath;
        row["price"] = m.value("price");
        row["usage"] = m.value("usage");
        out.append(row);
    }
    return out;
}

bool PreRecordManager::saveProducts(const QVariantList &list)
{
    AppDb &db = AppDb::instance();
    QVector<QVariantMap> existing = db.getOfferingsTemplateList();
    QSet<int> keptIx;

    for (const QVariant &v : list) {
        QVariantMap row = v.toMap();
        int ix = row.value("IX").toInt();
        QString name = row.value("name").toString();
        QString photoPath = row.value("photoPath").toString().trimmed();
        QString price = row.value("price").toString();
        QString usage = row.value("usage").toString();

        QString dbPhotoPath = photoPath;
        if (!photoPath.isEmpty()) {
            const QString appDir = QCoreApplication::applicationDirPath();
            const QString normPath = QDir::fromNativeSeparators(photoPath);
            const bool alreadyUnderOfferings = normPath.contains("/offerings/");
            QFileInfo fi(photoPath);
            if (alreadyUnderOfferings && fi.exists()) {
                dbPhotoPath = "offerings/" + fi.fileName();
            } else {
                QString copied = copyOfferingsImageToAppDir(photoPath, ix);
                if (!copied.isEmpty()) dbPhotoPath = copied;
            }
        }

        if (ix > 0) {
            if (!db.updateOfferingsTemplate(ix, name, dbPhotoPath, price, usage)) return false;
            keptIx.insert(ix);
        } else {
            int newIx = db.insertOfferingsTemplate(name, dbPhotoPath, price, usage);
            if (newIx > 0) keptIx.insert(newIx);
        }
    }

    for (const QVariantMap &row : existing) {
        int ix = row.value("IX").toInt();
        if (ix > 0 && !keptIx.contains(ix))
            db.deleteOfferingsTemplate(ix);
    }
    return true;
}

QVariantList PreRecordManager::getReportOfferings(int reportIndex, int tier) const
{
    if (reportIndex < 0 || reportIndex > 8 || tier < 0 || tier > 2) return QVariantList();
    int rt = reportTypeByIndex(reportIndex);
    int rl = reportLevelByTier(tier);
    int reportIx = AppDb::instance().getReportTemplateIx(rt, rl);
    if (reportIx <= 0) return QVariantList();
    QVector<int> ixs = AppDb::instance().getOfferingIxsForReportIx(reportIx);
    QVariantList out;
    for (int i : ixs) out.append(i);
    return out;
}

bool PreRecordManager::setReportOfferings(int reportIndex, int tier, const QVariantList &offeringIxs)
{
    if (reportIndex < 0 || reportIndex > 8 || tier < 0 || tier > 2) return false;
    int rt = reportTypeByIndex(reportIndex);
    int rl = reportLevelByTier(tier);
    int reportIx = AppDb::instance().getReportTemplateIx(rt, rl);
    if (reportIx <= 0) return false;
    QVector<int> ixs;
    for (const QVariant &v : offeringIxs) {
        int i = v.toInt();
        if (i > 0) ixs.append(i);
    }
    return AppDb::instance().setOfferingsForReportIx(reportIx, ixs);
}

bool PreRecordManager::save()
{
    AppDb &db = AppDb::instance();
    while (m_reportSuggestions.size() < 9) {
        QVariantMap m;
        m["good"] = m["medium"] = m["bad"] = QString();
        m_reportSuggestions.append(m);
    }
    for (int i = 0; i < 9; ++i) {
        int rt = reportTypeByIndex(i);
        QVariantMap m = m_reportSuggestions.at(i).toMap();
        if (!db.setReportTemplateMemo(rt, 30, m.value("good").toString())) return false;
        if (!db.setReportTemplateMemo(rt, 20, m.value("medium").toString())) return false;
        if (!db.setReportTemplateMemo(rt, 10, m.value("bad").toString())) return false;
    }
    return saveProducts(m_products);
}
