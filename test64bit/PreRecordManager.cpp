#include "PreRecordManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static const char* PRERECORD_FILENAME = "prerecord.json";

// 默认 9 个分量报告标签（与 customerReport.reportLabels 一致）
static const QStringList defaultReportLabels = {
    "毛 孔", "粉 刺", "深层色斑", "浅层色斑",
    "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告"
};

PreRecordManager::PreRecordManager(QObject *parent)
    : QObject(parent)
{
    // 默认 9 条空建议
    for (const QString &label : defaultReportLabels) {
        m_reportSuggestions.append(QString());
    }
    load();
}

QString PreRecordManager::configPath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(PRERECORD_FILENAME);
}

bool PreRecordManager::load()
{
    QFile f(configPath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject())
        return false;

    QJsonObject root = doc.object();

    if (root.contains("reportSuggestions") && root["reportSuggestions"].isArray()) {
        m_reportSuggestions = root["reportSuggestions"].toArray().toVariantList();
        while (m_reportSuggestions.size() < 9)
            m_reportSuggestions.append(QString());
        while (m_reportSuggestions.size() > 9)
            m_reportSuggestions.removeLast();
    }

    if (root.contains("products") && root["products"].isArray()) {
        m_products = root["products"].toArray().toVariantList();
    }

    return true;
}

QVariantList PreRecordManager::getReportSuggestions() const
{
    return m_reportSuggestions;
}

void PreRecordManager::setReportSuggestions(const QVariantList &list)
{
    m_reportSuggestions.clear();
    for (int i = 0; i < 9 && i < list.size(); ++i)
        m_reportSuggestions.append(list.at(i));
    while (m_reportSuggestions.size() < 9)
        m_reportSuggestions.append(QString());
}

QVariantList PreRecordManager::getProducts() const
{
    return m_products;
}

void PreRecordManager::setProducts(const QVariantList &list)
{
    m_products = list;
}

bool PreRecordManager::save()
{
    QJsonArray arrSuggestions = QJsonArray::fromVariantList(m_reportSuggestions);
    QJsonArray arrProducts;
    for (const QVariant &v : m_products) {
        QVariantMap m = v.toMap();
        QJsonObject o;
        o["name"] = m.value("name").toString();
        o["photoPath"] = m.value("photoPath").toString();
        o["price"] = m.value("price").toString();
        o["usage"] = m.value("usage").toString();
        arrProducts.append(o);
    }

    QJsonObject root;
    root["reportSuggestions"] = arrSuggestions;
    root["products"] = arrProducts;

    QFile f(configPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    f.close();
    return true;
}
