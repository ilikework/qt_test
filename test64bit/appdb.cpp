#include "AppDb.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>

static const char* DB_FILENAME = "MMFace_.db";

AppDb& AppDb::instance()
{
    static AppDb inst;
    return inst;
}

AppDb::AppDb(QObject* parent) : QObject(parent)
{
    openDb();
}

AppDb::~AppDb() {
    if (m_db.isValid()) {
        const auto name = m_db.connectionName();
        m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(name);
    }
}

bool AppDb::openDb()
{
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QString dbPath = QDir(exeDir).filePath(DB_FILENAME);

    const QString connName = "MMFace_singleton";
    if (QSqlDatabase::contains(connName))
        m_db = QSqlDatabase::database(connName);
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE", connName);

    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    m_lastError.clear();
    return true;
}

bool AppDb::exec(const QString& sql, const QVariantList& binds) {
    if (!m_db.isOpen()) { m_lastError = "DB not open"; return false; }

    QSqlQuery q(m_db);
    if (!q.prepare(sql)) { m_lastError = q.lastError().text(); return false; }
    for (int i=0;i<binds.size();++i) q.bindValue(i, binds[i]);

    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    m_lastError.clear();
    return true;
}
bool AppDb::isOpen() const
{
   return m_db.isOpen();
}

QString AppDb::lastErrorText() const
{
    return m_lastError;
}

QVector<QVariantMap> AppDb::select(const QString& sql, const QVariantList& binds)
{
    QVector<QVariantMap> rows;
    if (!m_db.isOpen()) { m_lastError = "DB not open"; return rows; }

    QSqlQuery q(m_db);
    if (!q.prepare(sql)) { m_lastError = q.lastError().text(); return rows; }
    for (int i=0;i<binds.size();++i) q.bindValue(i, binds[i]);

    if (!q.exec()) { m_lastError = q.lastError().text(); return rows; }

    const QSqlRecord rec = q.record();
    while (q.next()) {
        QVariantMap row;
        for (int i=0;i<rec.count();++i) row.insert(rec.fieldName(i), q.value(i));
        rows.push_back(std::move(row));
    }
    m_lastError.clear();
    return rows;
}

bool AppDb::begin()
{
    return m_db.transaction();
}

bool AppDb::commit()
{
    return m_db.commit();
}

bool AppDb::rollback()
{
    return m_db.rollback();
}

QVariantList AppDb::cameraConfigList(const int  nCameraSeries, const QString& configType)
{
    QVariantList out;

    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return out;
    }

    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        SELECT ConfigText, ConfigValue
        FROM T_CameraConfig
        WHERE Series=? AND ConfigType=?
        ORDER BY OrderID
    )SQL");

    q.addBindValue(nCameraSeries);
    q.addBindValue(configType);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }

    while (q.next()) {
        QVariantMap item;
        item.insert("text",  q.value(0).toString());
        item.insert("value", q.value(1));
        out.push_back(item);
    }

    m_lastError.clear();
    return out;
}

bool AppDb::loadCaptureSettingBySeries(const int nCameraSeries, CaptureSetting& out)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        SELECT
            Series,
            ImageSize,
            ImageQuality,

            RGB_ISO,
            RGB_ExposureTime,
            RGB_Aperture,
            RGB_WB,

            UV_ISO,
            UV_ExposureTime,
            UV_Aperture,
            UV_WB,

            PL_ISO,
            PL_ExposureTime,
            PL_Aperture,
            PL_WB,

            NPL_ISO,
            NPL_ExposureTime,
            NPL_Aperture,
            NPL_WB,

            Gender
        FROM T_CaptureSetting
        WHERE Series = ?
        LIMIT 1
    )SQL");

    q.addBindValue(nCameraSeries);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    if (!q.next()) {
        m_lastError = QString("No T_CaptureSetting record for Series=%1").arg(nCameraSeries);
        return false;
    }

    int c = 0;
    out.series = q.value(c++).toInt();

    out.imageSize = q.value(c++).toInt();
    out.imageQuality = q.value(c++).toInt();

    out.rgb_iso = q.value(c++).toInt();
    out.rgb_exposureTime = q.value(c++).toInt();
    out.rgb_aperture = q.value(c++).toInt();
    out.rgb_wb = q.value(c++).toInt();

    out.uv_iso = q.value(c++).toInt();
    out.uv_exposureTime = q.value(c++).toInt();
    out.uv_aperture = q.value(c++).toInt();
    out.uv_wb = q.value(c++).toInt();

    out.pl_iso = q.value(c++).toInt();
    out.pl_exposureTime = q.value(c++).toInt();
    out.pl_aperture = q.value(c++).toInt();
    out.pl_wb = q.value(c++).toInt();

    out.npl_iso = q.value(c++).toInt();
    out.npl_exposureTime = q.value(c++).toInt();
    out.npl_aperture = q.value(c++).toInt();
    out.npl_wb = q.value(c++).toInt();

    out.gender = q.value(c++).toInt();

    m_lastError.clear();
    return true;
}

void AppDb::CaptureSettingUpdate(const int nCameraSeries, const QString& column, const int value)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        //return false;
    }

    const QString sql = QString("UPDATE T_CaptureSetting SET %1=? WHERE Series=?").arg(column);
    if (!exec(sql, { value, nCameraSeries })) {
        qWarning() << "Update failed:" << lastErrorText();
    }



}

int AppDb::GetNextGroupID(const QString &CustomID)
{
   int  ret = 1;
    if (!m_db.isOpen())
   {
        m_lastError = "DB not open";
        return 0;
    }

    QSqlQuery q(m_db);
    const QString sql = QString("SELECT Max(Photo_ID) AS PhotoID FROM T_Customers_FacePhoto WHERE Cust_ID=%1").arg(CustomID);
    q.prepare(sql);
    if (!q.exec(sql)) {
        qWarning() << "Update failed:" << lastErrorText();
    }

    if (!q.next()) {
        m_lastError = QString("No T_Customers_FacePhoto record ");
        return ret;
    }

    int c = 0;
    ret = q.value(c++).toInt();
    ret++;

    return ret;
}

