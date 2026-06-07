#pragma once

#include <QObject>
#include <QVariantMap>
#include <QVariantList>

class CustomerReportManager : public QObject
{
    Q_OBJECT
public:
    explicit CustomerReportManager(QObject *parent = nullptr);

    // reportIndex 0..8；有已保存记录时从 T_Report_Main / T_Report_Offering 读取，否则从模板表读取
    Q_INVOKABLE QVariantMap loadReport(const QString &custId, int groupId, int reportIndex);
    // offeringIxs 为 T_Offerings_Template.IX 列表
    Q_INVOKABLE bool saveReport(const QString &custId, int groupId, int reportIndex,
                                int tier, const QString &memo, const QVariantList &offeringIxs);
    Q_INVOKABLE QString lastError() const { return m_lastError; }

private:
    static int reportTypeByIndex(int index);
    static int photoIdByReportIndex(int index);
    static int reportLevelByTier(int tier);
    static int tierByReportLevel(int level);

    QString m_lastError;
};
