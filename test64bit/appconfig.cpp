#include "AppConfig.h"
#include "MM_Const_Define.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

static const char* CONFIG_FILENAME = "MMFace_.json";

AppConfig& AppConfig::instance()
{
    static AppConfig inst;
    return inst;
}

AppConfig::AppConfig(QObject* parent)
    : QObject(parent)
{
    load();
}

QString AppConfig::configPath() const
{
    const QString dir = QCoreApplication::applicationDirPath();
    return QDir(dir).filePath(CONFIG_FILENAME);
}

bool AppConfig::load()
{
    QFile f(configPath());
    if (!f.exists()) {
        // 文件不存在：创建一个默认的
        m_root = QJsonObject{
            { "camera", "" },
            { "WaitForConnectCamera", 5000 }
        };
        save();
        return true;
    }

    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isObject()) {
        return false;
    }

    m_root = doc.object();

    return true;
}

bool AppConfig::save()
{
    QFile f(configPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QJsonDocument doc(m_root);
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

bool AppConfig::reload()
{
    return load();
}

int AppConfig::CameraSeries() const
{
    return m_root.value("CameraSeries").toInt();
}

QString AppConfig::CameraDll() const
{
    return m_root.value("CameraDll").toString();
}

int AppConfig::WaitForConnectCamera() const
{
    const int waitMs = m_root.value("WaitForConnectCamera").toInt(5000);
    return waitMs > 0 ? waitMs : 5000;
}

int AppConfig::GetBeforeInterval(const QString & capture_type) const
{
    if(capture_type=="RGB")
    {
        return BeforeRGBShootInterval();
    }
    else if(capture_type=="UV")
    {
        return BeforeUVShootInterval();
    }
    else if(capture_type=="PL")
    {
        return BeforePLShootInterval();
    }
    else if(capture_type=="NPL")
    {
        return BeforeNPLShootInterval();
    }
    return 0;
}

int AppConfig::GetAfterInterval(const QString & capture_type) const
{
    if(capture_type=="RGB")
    {
        return AfterRGBShootInterval();
    }
    else if(capture_type=="UV")
    {
        return AfterUVShootInterval();
    }
    else if(capture_type=="PL")
    {
        return AfterPLShootInterval();
    }
    else if(capture_type=="NPL")
    {
        return AfterNPLShootInterval();
    }
    return 0;

}


int AppConfig::BeforeRGBShootInterval() const
{
    int ret = 0;

    if (m_root.contains("BeforeRGBShootInterval") &&
        m_root.value("BeforeRGBShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("BeforeRGBShootInterval").toInt();
    }

    return ret;
}


int AppConfig::AfterRGBShootInterval() const
{
    int ret = 0;

    if (m_root.contains("AfterRGBShootInterval") &&
        m_root.value("AfterRGBShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("AfterRGBShootInterval").toInt();
    }

    return ret;
}

int AppConfig::BeforeUVShootInterval() const
{
    int ret = 0;

    if (m_root.contains("BeforeUVShootInterval") &&
        m_root.value("BeforeUVShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("BeforeUVShootInterval").toInt();
    }

    return ret;
}

int AppConfig::AfterUVShootInterval() const
{
    int ret = 0;

    if (m_root.contains("AfterUVShootInterval") &&
        m_root.value("AfterUVShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("AfterUVShootInterval").toInt();
    }

    return ret;
}

int AppConfig::BeforePLShootInterval() const
{
    int ret = 0;

    if (m_root.contains("BeforePLShootInterval") &&
        m_root.value("BeforePLShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("BeforePLShootInterval").toInt();
    }

    return ret;
}

int AppConfig::AfterPLShootInterval() const
{
    int ret = 0;

    if (m_root.contains("AfterPLShootInterval") &&
        m_root.value("AfterPLShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("AfterPLShootInterval").toInt();
    }

    return ret;
}

int AppConfig::BeforeNPLShootInterval() const
{
    int ret = 0;

    if (m_root.contains("BeforeNPLShootInterval") &&
        m_root.value("BeforeNPLShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("BeforeNPLShootInterval").toInt();
    }

    return ret;
}

int AppConfig::AfterNPLShootInterval() const
{
    int ret = 0;

    if (m_root.contains("AfterNPLShootInterval") &&
        m_root.value("AfterNPLShootInterval").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("AfterNPLShootInterval").toInt();
    }

    return ret;
}

QString AppConfig::GrayCreateFrom() const
{
    QString ret = "UV";

    if (m_root.contains("GrayCreateFrom") &&
        m_root.value("GrayCreateFrom").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("GrayCreateFrom").toString();
    }

    return ret;
}

QString AppConfig::RedCreateFrom() const
{
    QString ret = "NPL";

    if (m_root.contains("RedCreateFrom") &&
        m_root.value("RedCreateFrom").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("RedCreateFrom").toString();
    }

    return ret;

}

QString AppConfig::BrownCreateFrom() const
{
    QString ret = "UV";

    if (m_root.contains("BrownCreateFrom") &&
        m_root.value("BrownCreateFrom").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("BrownCreateFrom").toString();
    }

    return ret;

}

QString AppConfig::WholeCreateFrom() const
{
    QString ret = "PL";

    if (m_root.contains("WholeCreateFrom") &&
        m_root.value("WholeCreateFrom").isDouble())   // JSON 里数字都是 double
    {
        ret = m_root.value("WholeCreateFrom").toString();
    }

    return ret;

}

QString AppConfig::FaceReconExePath() const
{
    if (m_root.contains("FaceReconExePath")) {
        QString p = m_root.value("FaceReconExePath").toString();
        if (!p.isEmpty())
            return p;
    }
    return QString("D:/3D/FaceReconCPU/FaceReconCPU.exe");
}

namespace {

QJsonObject analyseRoot(const QJsonObject &root)
{
    return root.value(QStringLiteral("Analyse")).toObject();
}

QJsonObject analyseSections(const QJsonObject &root)
{
    return analyseRoot(root).value(QStringLiteral("Sections")).toObject();
}

int jsonInt(const QJsonObject &obj, const char *key, int fallback)
{
    if (!obj.contains(key))
        return fallback;
    return obj.value(QLatin1String(key)).toInt(fallback);
}

double jsonDouble(const QJsonObject &obj, const char *key, double fallback)
{
    if (!obj.contains(key))
        return fallback;
    return obj.value(QLatin1String(key)).toDouble(fallback);
}

} // namespace

QString AppConfig::analyseAgeBand(int ageYears) const
{
    if (ageYears < 20)
        return QStringLiteral("10");
    if (ageYears < 30)
        return QStringLiteral("20");
    if (ageYears < 40)
        return QStringLiteral("30");
    if (ageYears < 50)
        return QStringLiteral("40");
    if (ageYears < 60)
        return QStringLiteral("50");
    if (ageYears < 70)
        return QStringLiteral("60");
    return QStringLiteral("70");
}

QString AppConfig::analyseSectionBaseForReportType(int reportType) const
{
    switch (reportType) {
    case 1:
        return QStringLiteral("RGBPores");
    case 2:
        return QStringLiteral("UVAcne");
    case 3:
        return QStringLiteral("PLSpots");
    case 4:
        return QStringLiteral("PLSpots");
    case 5:
        return QStringLiteral("RGBWrinkle");
    case 6:
        return QStringLiteral("RGBEvenness");
    case 7:
        return QStringLiteral("BROWNEvenness");
    case 8:
        return QStringLiteral("RGBMoisture");
    default:
        return QStringLiteral("RGBPores");
    }
}

QString AppConfig::analyseSectionBaseForJob(int analyseFunction, const QString &capType) const
{
    switch (analyseFunction) {
    case MM_ANALYSE_PORES:
        return QStringLiteral("RGBPores");
    case MM_ANALYSE_ACNES:
        return QStringLiteral("UVAcne");
    case MM_ANALYSE_SPOTS:
        if (capType == MM_BROWN)
            return QStringLiteral("REDSpots");
        if (capType == MM_UV)
            return QStringLiteral("UVSpots");
        return QStringLiteral("PLSpots");
    case MM_ANALYSE_WRINKLE:
        return QStringLiteral("RGBWrinkle");
    case MM_ANALYSE_EVENNESS:
        return QStringLiteral("RGBEvenness");
    default:
        return analyseSectionBaseForReportType(analyseFunction);
    }
}

AnalyseSectionParams AppConfig::analyseSectionParams(const QString &sectionBase, int ageYears) const
{
    auto parseSection = [](const QJsonObject &section, AnalyseSectionParams defaults) {
        AnalyseSectionParams p = defaults;
        p.femaleMin = jsonInt(section, "FemaleMin", p.femaleMin);
        p.femaleMax = jsonInt(section, "FemaleMax", p.femaleMax);
        p.maleMin = jsonInt(section, "MaleMin", p.maleMin);
        p.maleMax = jsonInt(section, "MaleMax", p.maleMax);
        p.minArea = jsonInt(section, "MinArea", p.minArea);
        p.maxArea = jsonInt(section, "MaxArea", p.maxArea);
        p.conVal = jsonInt(section, "ConVal", p.conVal);
        p.maleGain = jsonDouble(section, "MaleGain", p.maleGain);
        p.femaleGain = jsonDouble(section, "FemaleGain", p.femaleGain);
        if (p.maleGain <= 0.0)
            p.maleGain = 1.0;
        if (p.femaleGain <= 0.0)
            p.femaleGain = 1.0;
        return p;
    };

    AnalyseSectionParams p;
    const QJsonObject sections = analyseSections(m_root);
    const QString sectionKey = sectionBase + analyseAgeBand(ageYears);
    QJsonObject section = sections.value(sectionKey).toObject();
    if (section.isEmpty() && ageYears != 30)
        section = sections.value(sectionBase + QStringLiteral("30")).toObject();
    if (section.isEmpty())
        return p;
    return parseSection(section, p);
}

void AppConfig::analyseMinMaxForJob(int analyseFunction, const QString &capType, int ageYears,
                                    int gender, int *outMin, int *outMax) const
{
    const QString base = analyseSectionBaseForJob(analyseFunction, capType);
    const AnalyseSectionParams p = analyseSectionParams(base, ageYears);
    const bool male = (gender != 0);
    if (outMin)
        *outMin = male ? p.maleMin : p.femaleMin;
    if (outMax)
        *outMax = male ? p.maleMax : p.femaleMax;
    if (outMin && *outMin <= 0 && analyseFunction == MM_ANALYSE_PORES)
        *outMin = male ? 75 : 120;
    if (outMax && *outMax <= 0 && analyseFunction == MM_ANALYSE_PORES)
        *outMax = male ? 125 : 130;
}

void AppConfig::analyseRunOptionsForJob(int analyseFunction, const QString &capType, int ageYears,
                                        int *outConVal, int *outMinArea, int *outMaxArea) const
{
    const AnalyseSectionParams p = analyseSectionParams(
        analyseSectionBaseForJob(analyseFunction, capType), ageYears);
    if (outConVal)
        *outConVal = p.conVal;
    if (outMinArea)
        *outMinArea = p.minArea;
    if (outMaxArea)
        *outMaxArea = p.maxArea;
}

double AppConfig::correctPercent(double magicFacePercent, int reportType, int gender,
                                 int ageYears) const
{
    if (reportType == 8)
        return magicFacePercent;

    const AnalyseSectionParams p = analyseSectionParams(
        analyseSectionBaseForReportType(reportType), ageYears);
    const double fgain = (gender != 0) ? p.maleGain : p.femaleGain;
    double percent = magicFacePercent;
    if (percent < 0.0)
        percent = 0.0;

    const double fvalue = std::sqrt(percent) * fgain;
    if (fvalue <= 50.0)
        return fvalue;

    double fNum = 0.0;
    double fCount = (fvalue - 50.0) / 10.0;
    int nCount = static_cast<int>(fCount);
    const double fTail = fCount - static_cast<double>(nCount);
    if (nCount > 60)
        nCount = 60;

    int i = 0;
    if (nCount - 1 > 0) {
        for (i = 0; i <= nCount - 1; ++i)
            fNum += std::pow(0.82, i + 1) * 10.0;
        i = nCount - 1;
    } else {
        i = 0;
    }

    fNum += fTail * std::pow(0.82, i + 1) * 10.0;
    if (nCount == 0)
        fNum += fTail * 8.2;

    if (fNum == 0.0)
        return fvalue;
    return fNum + 50.0;
}

double AppConfig::displayScoreFromRawPercent(int rawPercent, int reportType, int gender,
                                            int ageYears) const
{
    const double magicFacePercent = rawPercent / 100.0;
    return correctPercent(magicFacePercent, reportType, gender, ageYears);
}

