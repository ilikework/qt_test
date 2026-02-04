#pragma once

#include <QObject>
#include <QVariantList>

class PreRecordManager : public QObject
{
    Q_OBJECT
public:
    explicit PreRecordManager(QObject *parent = nullptr);

    // 分量报告建议：从 T_Report_Template 读取，9 项与 reportLabels 对应，每项 { label, goodMemo, mediumMemo, badMemo }
    // reportIndex 0..8，tier 0=好 1=中 2=差 -> Report_LEVEL 30/20/10
    Q_INVOKABLE QVariantList getReportSuggestions() const;
    Q_INVOKABLE bool setReportMemo(int reportIndex, int tier, const QString &text);

    // 产品/服务：从 T_Offerings_Template 读取/写入，每项含 IX, name, photoPath, price, usage
    // photoPath 返回给 QML 时为可显示的完整路径（offerings/ 会拼上程序目录）
    Q_INVOKABLE QVariantList getProducts() const;
    Q_INVOKABLE bool saveProducts(const QVariantList &list);

    // 报告关联的产品：reportIndex 0..8，tier 0/1/2；offeringIxs 为 T_Offerings_Template 的 IX 列表
    Q_INVOKABLE QVariantList getReportOfferings(int reportIndex, int tier) const;
    Q_INVOKABLE bool setReportOfferings(int reportIndex, int tier, const QVariantList &offeringIxs);

    Q_INVOKABLE bool save();

private:
    static int reportTypeByIndex(int index);   // 0..7 -> 1..8, 8 -> 100
    static int reportLevelByTier(int tier);   // 0->30, 1->20, 2->10
    static QString copyOfferingsImageToAppDir(const QString &sourcePath, int offeringIx);

    QVariantList m_reportSuggestions;
    QVariantList m_products;
};
