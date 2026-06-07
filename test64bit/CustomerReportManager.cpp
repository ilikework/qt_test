#include "CustomerReportManager.h"
#include "AppDb.h"

int CustomerReportManager::reportTypeByIndex(int index)
{
    return index < 8 ? (index + 1) : 100;
}

int CustomerReportManager::photoIdByReportIndex(int index)
{
    return index < 8 ? (index + 1) : -1;
}

int CustomerReportManager::reportLevelByTier(int tier)
{
    if (tier == 0) return 30;
    if (tier == 1) return 20;
    return 10;
}

int CustomerReportManager::tierByReportLevel(int level)
{
    if (level == 30) return 0;
    if (level == 20) return 1;
    return 2;
}

CustomerReportManager::CustomerReportManager(QObject *parent)
    : QObject(parent)
{
}

QVariantMap CustomerReportManager::loadReport(const QString &custId, int groupId, int reportIndex)
{
    QVariantMap out;
    out["hasSaved"] = false;
    out["tier"] = 1;
    out["memo"] = QString();
    out["offeringIxs"] = QVariantList();

    m_lastError.clear();
    if (reportIndex < 0 || reportIndex > 8 || custId.isEmpty() || groupId <= 0)
        return out;

    AppDb &db = AppDb::instance();
    if (!db.ensureCustomerReportTables()) {
        m_lastError = db.lastErrorText();
        return out;
    }

    const int photoId = photoIdByReportIndex(reportIndex);
    const int reportType = reportTypeByIndex(reportIndex);

    int reportIx = 0;
    QString memo;
    int level = 0;
    if (!db.findCustomerReportMain(custId, groupId, photoId, reportType, reportIx, memo, level))
        return out;

    out["hasSaved"] = true;
    out["tier"] = tierByReportLevel(level);
    out["memo"] = memo;
    QVariantList ixs;
    for (int ix : db.getCustomerReportOfferings(reportIx))
        ixs.append(ix);
    out["offeringIxs"] = ixs;
    return out;
}

bool CustomerReportManager::saveReport(const QString &custId, int groupId, int reportIndex,
                                       int tier, const QString &memo, const QVariantList &offeringIxs)
{
    m_lastError.clear();
    if (reportIndex < 0 || reportIndex > 8 || custId.isEmpty() || groupId <= 0) {
        m_lastError = "invalid parameters";
        return false;
    }
    if (tier < 0 || tier > 2) {
        m_lastError = "invalid tier";
        return false;
    }

    AppDb &db = AppDb::instance();
    if (!db.ensureCustomerReportTables()) {
        m_lastError = db.lastErrorText();
        return false;
    }

    const int photoId = photoIdByReportIndex(reportIndex);
    const int reportType = reportTypeByIndex(reportIndex);
    const int level = reportLevelByTier(tier);

    QVector<int> ixs;
    for (const QVariant &v : offeringIxs) {
        const int i = v.toInt();
        if (i > 0)
            ixs.append(i);
    }

    const int reportIx = db.upsertCustomerReportMain(custId, groupId, photoId, reportType, level, memo);
    if (reportIx <= 0) {
        m_lastError = db.lastErrorText();
        return false;
    }
    if (!db.setCustomerReportOfferings(reportIx, ixs)) {
        m_lastError = db.lastErrorText();
        return false;
    }
    return true;
}
