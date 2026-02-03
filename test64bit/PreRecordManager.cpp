#include "PreRecordManager.h"
#include "AppDb.h"
#include <QSet>

// Report_Type: 1-8 对应 reportLabels[0..7]，100 对应综合报告
int PreRecordManager::reportTypeByIndex(int index)
{
    return index < 8 ? (index + 1) : 100;
}

PreRecordManager::PreRecordManager(QObject *parent)
    : QObject(parent)
{
}

QVariantList PreRecordManager::getReportSuggestions() const
{
    QVariantList out;
    AppDb &db = AppDb::instance();
    for (int i = 0; i < 9; ++i) {
        int rt = reportTypeByIndex(i);
        QVariantMap m;
        m["good"] = db.getReportTemplateMemo(rt, 30);
        m["medium"] = db.getReportTemplateMemo(rt, 20);
        m["bad"] = db.getReportTemplateMemo(rt, 10);
        out.append(m);
    }
    return out;
}

void PreRecordManager::setReportSuggestions(const QVariantList &list)
{
    m_reportSuggestions = list;
}

QVariantList PreRecordManager::getProducts() const
{
    QVariantList out;
    QVector<QVariantMap> rows = AppDb::instance().getOfferingsTemplateList();
    for (const QVariantMap &m : rows) {
        QVariantMap row;
        if (m.contains("IX")) row["IX"] = m["IX"];
        row["name"] = m.value("name");
        row["photoPath"] = m.value("photoPath");
        row["price"] = m.value("price");
        row["usage"] = m.value("usage");
        out.append(row);
    }
    return out;
}

void PreRecordManager::setProducts(const QVariantList &list)
{
    m_products = list;
}

QVariantList PreRecordManager::getLinkedOfferingIxs(int reportType, int reportLevel) const
{
    int reportIx = AppDb::instance().getReportTemplateIx(reportType, reportLevel);
    if (reportIx <= 0) return QVariantList();
    QVector<int> ixs = AppDb::instance().getOfferingIxsForReportIx(reportIx);
    QVariantList out;
    for (int i : ixs) out.append(i);
    return out;
}

bool PreRecordManager::setLinkedOfferingIxs(int reportType, int reportLevel, const QVariantList &offeringIxs)
{
    int reportIx = AppDb::instance().getReportTemplateIx(reportType, reportLevel);
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
    QVector<QVariantMap> existing = db.getOfferingsTemplateList();
    QSet<int> keptIx;
    for (const QVariant &v : m_products) {
        QVariantMap row = v.toMap();
        int ix = row.value("IX").toInt();
        QString name = row.value("name").toString();
        QString photoPath = row.value("photoPath").toString();
        QString price = row.value("price").toString();
        QString usage = row.value("usage").toString();
        if (ix > 0) {
            if (!db.updateOfferingsTemplate(ix, name, photoPath, price, usage)) return false;
            keptIx.insert(ix);
        } else {
            int newIx = db.insertOfferingsTemplate(name, photoPath, price, usage);
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
