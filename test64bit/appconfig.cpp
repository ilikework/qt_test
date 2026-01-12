#include "AppConfig.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

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
            { "camera", "" }
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

    qDebug() << "AfterRGBShootInterval ret=" << ret;
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

    qDebug() << "BeforeUVShootInterval ret=" << ret;
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
    qDebug() << "AfterUVShootInterval ret=" << ret;

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

    qDebug() << "BeforePLShootInterval ret=" << ret;
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

    qDebug() << "AfterPLShootInterval ret=" << ret;
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

    qDebug() << "BeforeNPLShootInterval ret=" << ret;
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

    qDebug() << "AfterNPLShootInterval ret=" << ret;
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

    qDebug() << "GrayCreateFrom ret=" << ret;
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

    qDebug() << "RedCreateFrom ret=" << ret;
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

    qDebug() << "BrownCreateFrom ret=" << ret;
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

    qDebug() << "WholeCreateFrom ret=" << ret;
    return ret;

}

