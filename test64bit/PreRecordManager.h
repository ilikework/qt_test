#pragma once

#include <QObject>
#include <QVariantList>

class PreRecordManager : public QObject
{
    Q_OBJECT
public:
    explicit PreRecordManager(QObject *parent = nullptr);

    // 分量报告建议：从 T_Report_Template 读取，9 项与 customerReport.reportLabels 对应，每项 { good, medium, bad }
    // Report_LEVEL 30=好 20=中 10=差。照片分析后按 scorePercent 取对应档位的 MEMO 和 offerings 生成报告。
    Q_INVOKABLE QVariantList getReportSuggestions() const;
    Q_INVOKABLE void setReportSuggestions(const QVariantList &list);

    // 产品/服务：从 T_Offerings_Template 读取/写入，每项含 IX, name, photoPath, price, usage
    Q_INVOKABLE QVariantList getProducts() const;
    Q_INVOKABLE void setProducts(const QVariantList &list);

    // 按报告槽位(Report_Type, Report_LEVEL) 获取/设置关联的 Offering_IX 列表（T_Report_Offerings_Template）
    Q_INVOKABLE QVariantList getLinkedOfferingIxs(int reportType, int reportLevel) const;
    Q_INVOKABLE bool setLinkedOfferingIxs(int reportType, int reportLevel, const QVariantList &offeringIxs);

    Q_INVOKABLE bool save();

private:
    static int reportTypeByIndex(int index); // 0..7 -> 1..8, 8 -> 100
    QVariantList m_reportSuggestions;
    QVariantList m_products;
};
