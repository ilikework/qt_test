#pragma once

#include <QObject>
#include <QVariantList>

class PreRecordManager : public QObject
{
    Q_OBJECT
public:
    explicit PreRecordManager(QObject *parent = nullptr);

    // 分量报告建议文字：9 项，与 reportLabels 对应
    Q_INVOKABLE QVariantList getReportSuggestions() const;
    Q_INVOKABLE void setReportSuggestions(const QVariantList &list);

    // 产品/服务列表：每项 { "name", "photoPath", "price", "usage" }
    Q_INVOKABLE QVariantList getProducts() const;
    Q_INVOKABLE void setProducts(const QVariantList &list);

    Q_INVOKABLE bool save();

private:
    QString configPath() const;
    bool load();

    QVariantList m_reportSuggestions;
    QVariantList m_products;
};
