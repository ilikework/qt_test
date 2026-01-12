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
    const QString sql = QString("SELECT Max(Group_ID) AS GroupID FROM T_Customers_FacePhoto WHERE Cust_ID=%1").arg(CustomID);
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

QString AppDb::GetNextCustomID()
{
    int  nRet = 1;
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return "";
    }

    QSqlQuery q(m_db);
    const QString sql = QString("SELECT Max(IX)  FROM T_Customers ");
    q.prepare(sql);
    if (!q.exec(sql)) {
        qWarning() << "Update failed:" << lastErrorText();
    }

    if (!q.next()) {
        m_lastError = QString("No T_Customers record ");

        return QString("%1").arg(nRet, 7, 10, QChar('0'));
    }

    int c = 0;
    nRet = q.value(c++).toInt();
    nRet++;

    return QString("%1").arg(nRet, 7, 10, QChar('0'));
}

bool AppDb::FindCustomers(const QString &filter, QVector<Customer> &customerList)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    customerList.clear(); // 建议先清空列表，防止重复添加

    QSqlQuery q(m_db);
    QString sql = R"(SELECT
                        IX, Cust_ID, Cust_Name, Cust_Gender,
                        Cust_Birthday, Cust_EditTime, Cust_Phone,
                        Cust_Addr, Cust_EMail, Cust_Photo, Cust_Des
                    FROM T_Customers)";

    // 如果有过滤条件
    if (!filter.isEmpty()) {
        sql += R"( WHERE Cust_Name LIKE :f
                   OR Cust_ID LIKE :f
                   OR Cust_Phone LIKE :f
                   OR Cust_Addr LIKE :f
                   OR Cust_EMail LIKE :f )";
    }

    q.prepare(sql);

    if (!filter.isEmpty()) {
        // 在这里处理通配符，例如输入 "123"，变成 "%123%"
        QString fuzzyFilter = QString("%%%1%%").arg(filter);
        q.bindValue(":f", fuzzyFilter);
    }

    if (!q.exec()) { // 不要传入 sql 字符串
        m_lastError = q.lastError().text();
        qWarning() << "T_Customers SELECT failed:" << m_lastError;
        return false;
    }

    // 预留空间可以提高 push_back 的效率（可选）
    // if (q.isSelect()) customerList.reserve(q.size());

    while (q.next()) {
        Customer r;
        r.IX            = q.value(0).toInt();
        r.Cust_ID       = q.value(1).toString();
        r.Cust_Name     = q.value(2).toString();
        r.Cust_Gender   = q.value(3).toInt();
        r.Cust_Birthday = q.value(4).toString();
        r.Cust_EditTime = q.value(5).toString();
        r.Cust_Phone    = q.value(6).toString();
        r.Cust_Addr     = q.value(7).toString();
        r.Cust_EMail    = q.value(8).toString();
        r.Cust_Photo    = q.value(9).toString();
        r.Cust_Des      = q.value(10).toString();

        customerList.push_back(std::move(r));
    }

    return true;
}

bool AppDb::AddCustomer( Customer &customer)
{
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return false;
    }
    QSqlQuery q(m_db);
    // ⚠️ 按你的 T_Customers 实际列名改这里
    q.prepare(R"SQL(
        INSERT INTO T_Customers(
                    Cust_ID,
                    Cust_Name,
                    Cust_Gender,
                    Cust_Birthday,
                    Cust_EditTime,
                    Cust_Phone,
                    Cust_Addr,
                    Cust_EMail,
                    Cust_Photo,
                    Cust_Des
        )
        VALUES(
            :id, :name, :gender, :birthday, :editdate, :phone, :address, :email, :photo,:des
        )
    )SQL");

    customer.Cust_ID = GetNextCustomID();
    q.bindValue(":id", customer.Cust_ID);
    q.bindValue(":name", customer.Cust_Name);
    q.bindValue(":gender", customer.Cust_Gender);
    q.bindValue(":birthday", customer.Cust_Birthday);       // "yyyy-MM-dd"
    q.bindValue(":editdate", customer.Cust_EditTime);               // "yyyy-MM-dd"
    q.bindValue(":phone", customer.Cust_Phone);
    q.bindValue(":address",customer.Cust_Addr);
    q.bindValue(":email", customer.Cust_EMail);
    q.bindValue(":photo", customer.Cust_Photo);
    q.bindValue(":des", customer.Cust_Des);

    if (!q.exec()) {
        qWarning() << "addCustomer INSERT failed:" << q.lastError().text();
        return false;
    }
    customer.IX = q.lastInsertId().toInt();
    return true;
}

bool AppDb::DeleteCustomer(const QString &CustomID)
{
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return false;
    }
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM T_Customers WHERE Cust_ID = :id");
    q.bindValue(":id", CustomID);
    if (!q.exec())
    {
        m_lastError = q.lastError().text();
        qWarning() << "T_Customers DELETE failed:" << m_lastError;
        return false;
    }

    // 额外检查：如果没有报错，但也没有删掉任何行（比如 ID 不存在）
    if (q.numRowsAffected() == 0) {
        qDebug() << "Warning: No customer found with ID:" << CustomID;
    }
    return true;

}

bool AppDb::editCustomer(const Customer &customer)
{
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return false;
    }
    QSqlQuery q(m_db);

    q.prepare(R"SQL(
        UPDATE T_Customers SET
            Cust_ID = :id,
            Cust_Name = :name,
            Cust_Gender = :gender,
            Cust_Birthday = :birthday,
            Cust_EditTime = :editdate,
            Cust_Phone = :phone,
            Cust_Addr = :address,
            Cust_EMail = :email,
            Cust_Photo = :photo,
            Cust_Des = :des
        WHERE IX = :ix
    )SQL");

    q.bindValue(":id", customer.Cust_ID);
    q.bindValue(":name", customer.Cust_Name);
    q.bindValue(":gender", customer.Cust_Gender);
    q.bindValue(":birthday", customer.Cust_Birthday);       // "yyyy-MM-dd"
    q.bindValue(":editdate", customer.Cust_EditTime);               // "yyyy-MM-dd"
    q.bindValue(":phone", customer.Cust_Phone);
    q.bindValue(":address",customer.Cust_Addr);
    q.bindValue(":email", customer.Cust_EMail);
    q.bindValue(":photo", customer.Cust_Photo);
    q.bindValue(":des", customer.Cust_Des);
    q.bindValue(":ix", customer.IX);


    if (!q.exec()) {
        qWarning() << "editCustomer UPDATE failed:" << q.lastError().text();
        return false;
    }

    // row 没更新：可能 id 不存在
    if (q.numRowsAffected() <= 0) {
        qWarning() << "editCustomer: no rows affected, ix not found?" << customer.IX;
        // 你可以选择 return false; 或者当作成功
        return false;
    }

    return true;
}

bool AppDb::updateCustomerPhoto(const int IX, const QString &photo)
{
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return false;
    }
    QSqlQuery q(m_db);

    q.prepare(R"SQL(
        UPDATE T_Customers SET
            Cust_Photo = :photo
        WHERE IX = :ix
    )SQL");

    q.bindValue(":photo", photo);
    q.bindValue(":ix", IX);


    if (!q.exec()) {
        qWarning() << "editCustomer UPDATE failed:" << q.lastError().text();
        return false;
    }

    // row 没更新：可能 id 不存在
    if (q.numRowsAffected() <= 0) {
        qWarning() << "editCustomer: no rows affected, ix not found?" << IX;
        // 你可以选择 return false; 或者当作成功
        return false;
    }

    return true;
}
