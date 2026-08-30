#pragma once
#include <QObject>
#include <QJsonObject>
#include <QString>

struct AnalyseSectionParams {
    int femaleMin = 0;
    int femaleMax = 0;
    int maleMin = 75;
    int maleMax = 125;
    int minArea = 30;
    int maxArea = 100;
    int conVal = 80;
    double maleGain = 1.0;
    double femaleGain = 1.0;
};

class AppConfig : public QObject {
    Q_OBJECT

public:
    static AppConfig& instance();

    int CameraSeries() const;
    QString CameraDll() const;
    int GetBeforeInterval(const QString & capture_type) const;
    int GetAfterInterval(const QString & capture_type) const;
    int WaitForConnectCamera() const;
    int BeforeRGBShootInterval() const;
    int AfterRGBShootInterval() const;
    int BeforeUVShootInterval() const;
    int AfterUVShootInterval() const;
    int BeforePLShootInterval() const;
    int AfterPLShootInterval() const;
    int BeforeNPLShootInterval() const;
    int AfterNPLShootInterval() const;
    QString GrayCreateFrom() const;
    QString RedCreateFrom() const;
    QString BrownCreateFrom() const;
    QString WholeCreateFrom() const;
    QString FaceReconExePath() const;

    /** analysis.ini 年龄档：10/20/…/70 */
    QString analyseAgeBand(int ageYears) const;
    /** Report_Type 1..8 → ini section 基名（与 TC30 Delphi 一致） */
    QString analyseSectionBaseForReportType(int reportType) const;
    /** 分析任务 → ini section 基名 */
    QString analyseSectionBaseForJob(int analyseFunction, const QString &capType) const;
    AnalyseSectionParams analyseSectionParams(const QString &sectionBase, int ageYears) const;
    void analyseMinMaxForJob(int analyseFunction, const QString &capType, int ageYears, int gender,
                             int *outMin, int *outMax) const;
    void analyseRunOptionsForJob(int analyseFunction, const QString &capType, int ageYears,
                                 int *outConVal, int *outMinArea, int *outMaxArea) const;
    /** 输入 MagicFace GetMAnalysePercent = rawPercent/100；输出图表刻度（TC30 correctPercent） */
    double correctPercent(double magicFacePercent, int reportType, int gender, int ageYears) const;
    double displayScoreFromRawPercent(int rawPercent, int reportType, int gender, int ageYears) const;

    bool save();   // 写回 MMFace_.json
    bool reload(); // 重新读

signals:

private:
    explicit AppConfig(QObject* parent = nullptr);
    ~AppConfig() = default;

    Q_DISABLE_COPY_MOVE(AppConfig)

    bool load();
    QString configPath() const;

private:
    QJsonObject m_root;
};
