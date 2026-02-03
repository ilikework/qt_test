#include "AppDb.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>
#include "MM_Const_Define.h"

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
    closeDb();
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

void AppDb::closeDb()
{
    if (m_db.isValid()) {
        const auto name = m_db.connectionName();
        m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(name);
    }
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
    const QString sql = QString("SELECT Max(Group_ID) AS GroupID FROM T_Customers_FacePhoto WHERE Cust_ID=:id");
    q.prepare(sql);
    q.bindValue(":id", CustomID);
    if (!q.exec()) {
        qWarning() << "SELECT failed:" << lastErrorText();
    }

    if (!q.next()) {
        m_lastError = QString("No T_Customers_FacePhoto record ");
        return ret;
    }

    ret = q.value(0).toInt();
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

bool AppDb::addCustomer( Customer &customer)
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
            :id, :name, :gender, :birthday, CURRENT_TIMESTAMP, :phone, :address, :email, :photo,:des
        )
    )SQL");

    customer.Cust_ID = GetNextCustomID();
    q.bindValue(":id", customer.Cust_ID);
    q.bindValue(":name", customer.Cust_Name);
    q.bindValue(":gender", customer.Cust_Gender);
    q.bindValue(":birthday", customer.Cust_Birthday);       // "yyyy-MM-dd"
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

bool AppDb::addPhoto(FacePhoto &photo)
{
    if (!m_db.isOpen())
    {
        m_lastError = "DB not open";
        return false;
    }
    QSqlQuery q(m_db);
    // ⚠️ 按你的 T_Customers 实际列名改这里
    q.prepare(R"SQL(
                INSERT
                INTO "T_Customers_FacePhoto" (
                  IX
                  ,Cust_ID
                  ,Photo_CapType
                  ,Photo_DirType
                  ,Group_ID
                  ,Photo_ID
                  ,Photo_Name
                  ,Photo_EditTime
                  ,photo_iso
                  ,photo_wb
                  ,photo_aperture
                  ,photo_exposuretime
                  ,photo_comment
                )
                VALUES (
                  :IX
                  , :id
                  , :CapType
                  , :DirType
                  , :GroupID
                  , :PhotoID
                  , :Name
                  , CURRENT_TIMESTAMP
                  , :iso
                  , :wb
                  , :aperture
                  , :exposuretime
                  , :comment
                )
            )SQL");

    q.bindValue(":id", photo.Cust_ID);
    q.bindValue(":CapType", photo.Photo_CapType);
    q.bindValue(":DirType", photo.Photo_DirType);
    q.bindValue(":GroupID", photo.Group_ID);
    q.bindValue(":PhotoID", photo.Photo_ID);
    q.bindValue(":Name", photo.Photo_Name);
    q.bindValue(":iso", photo.photo_iso);
    q.bindValue(":wb", photo.photo_wb);
    q.bindValue(":aperture", photo.photo_aperture);
    q.bindValue(":exposuretime", photo.photo_exposuretime);
    q.bindValue(":comment", photo.photo_comment);

    if (!q.exec()) {
        qWarning() << "addCustomer INSERT failed:" << q.lastError().text();
        return false;
    }

    photo.IX = q.lastInsertId().toInt();
    return true;
}

bool AppDb::findPhotoesbyCustomID(const QString &CustomID, QVector<FacePhoto> &photoList)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    photoList.clear(); // 建议先清空列表，防止重复添加

    QSqlQuery q(m_db);
    QString sql = R"(SELECT
                      IX
                      ,Cust_ID
                      ,Photo_CapType
                      ,Photo_DirType
                      ,Group_ID
                      ,Photo_ID
                      ,Photo_Name
                      ,Photo_EditTime
                      ,photo_iso
                      ,photo_wb
                      ,photo_aperture
                      ,photo_exposuretime
                      ,photo_comment
                    FROM
                      T_Customers_FacePhoto
                    WHERE Cust_ID = :id
                    )";
    q.prepare(sql);
    q.bindValue(":id",CustomID);

    if (!q.exec()) { // 不要传入 sql 字符串
        m_lastError = q.lastError().text();
        qWarning() << "T_Customers SELECT failed:" << m_lastError;
        return false;
    }


    while (q.next()) {
        FacePhoto p;
        p.IX                 = q.value(0).toInt();
        p.Cust_ID            = q.value(1).toString();
        p.Photo_CapType      = q.value(2).toString();
        p.Photo_DirType      = q.value(3).toString();
        p.Group_ID           = q.value(4).toInt();
        p.Photo_ID           = q.value(5).toInt();
        p.Photo_Name         = q.value(6).toString();
        p.Photo_EditTime     = q.value(7).toString();
        p.photo_iso          = q.value(8).toString();
        p.photo_wb           = q.value(9).toString();
        p.photo_aperture     = q.value(10).toString();
        p.photo_exposuretime = q.value(11).toString();
        p.photo_comment      = q.value(12).toString();

        photoList.push_back(std::move(p));
    }

    return true;
}

bool AppDb::findPhotoesbyCustomIDandGropuID(const QString &CustomID, const int nGroupID, QVector<FacePhoto> &photoList)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    photoList.clear(); // 建议先清空列表，防止重复添加

    QSqlQuery q(m_db);
    QString sql = R"(SELECT
                      IX
                      ,Cust_ID
                      ,Photo_CapType
                      ,Photo_DirType
                      ,Group_ID
                      ,Photo_ID
                      ,Photo_Name
                      ,Photo_EditTime
                      ,photo_iso
                      ,photo_wb
                      ,photo_aperture
                      ,photo_exposuretime
                      ,photo_comment
                    FROM
                      T_Customers_FacePhoto
                    WHERE Cust_ID = :id
                      AND Group_ID = :groupid
                    )";
    q.prepare(sql);
    q.bindValue(":id",CustomID);
    q.bindValue(":groupid",nGroupID);

    if (!q.exec()) { // 不要传入 sql 字符串
        m_lastError = q.lastError().text();
        qWarning() << "T_Customers SELECT failed:" << m_lastError;
        return false;
    }


    while (q.next()) {
        FacePhoto p;
        p.IX                 = q.value(0).toInt();
        p.Cust_ID            = q.value(1).toString();
        p.Photo_CapType      = q.value(2).toString();
        p.Photo_DirType      = q.value(3).toString();
        p.Group_ID           = q.value(4).toInt();
        p.Photo_ID           = q.value(5).toInt();
        p.Photo_Name         = q.value(6).toString();
        p.Photo_EditTime     = q.value(7).toString();
        p.photo_iso          = q.value(8).toString();
        p.photo_wb           = q.value(9).toString();
        p.photo_aperture     = q.value(10).toString();
        p.photo_exposuretime = q.value(11).toString();
        p.photo_comment      = q.value(12).toString();

        photoList.push_back(std::move(p));
    }

    return true;
}

int AppDb::insertDrawInfo(int facePhotoIx, const QString& jsonInfo) {
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO T_FacePhoto_DrawInfo (FacePhoto_IX, Info, EditTime) "
                  "VALUES (?, ?, DATETIME('now', 'localtime'))");
    query.addBindValue(facePhotoIx);
    query.addBindValue(jsonInfo);

    if (!query.exec()) {
        qDebug() << "insertDrawInfo Error:" << query.lastError().text();
        return -1;
    }

    // 获取 SQLite 刚刚生成的自增主键 IX
    return query.lastInsertId().toInt();
}

bool AppDb::deleteDrawInfoByIX(int ix) {
    if (ix < 0) return false;

    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM T_FacePhoto_DrawInfo WHERE IX = ?");
    query.addBindValue(ix);

    if (!query.exec()) {
        qDebug() << "deleteDrawInfoByIX Error:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QPair<int, QString>> AppDb::getAllDrawInfos(int facePhotoIx) {
    QList<QPair<int, QString>> results;
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return results;
    }

    QSqlQuery query(m_db);

    // 按时间顺序或 IX 顺序查询所有属于该图片的标注
    query.prepare("SELECT IX, Info FROM T_FacePhoto_DrawInfo WHERE FacePhoto_IX = ? ORDER BY IX ASC");
    query.addBindValue(facePhotoIx);

    if (query.exec()) {
        while (query.next()) {
            int ix = query.value(0).toInt();
            QString info = query.value(1).toString();
            results.append(qMakePair(ix, info));
        }
    } else {
        qDebug() << "getAllDrawInfos Error:" << query.lastError().text();
    }

    return results;
}

QString AppDb::getTemplateInfo(const QString &dirType)
{
    if (!m_db.isOpen()) {
        m_lastError = "DB not open";
        return "";
    }
    QSqlQuery query(m_db);
    query.prepare("SELECT Info FROM T_FacePhoto_DrawInfo_Template WHERE Photo_DirType = :type");
    query.bindValue(":type", dirType);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

QString AppDb::getReportTemplateMemo(int reportType, int reportLevel)
{
    QVector<QVariantMap> rows = select(
        "SELECT MEMO FROM T_Report_Template WHERE Report_Type = ? AND Report_LEVEL = ? LIMIT 1",
        { reportType, reportLevel });
    if (rows.isEmpty()) return QString();
    QVariant v = rows.at(0).value("MEMO");
    return v.isNull() ? QString() : v.toString();
}

bool AppDb::setReportTemplateMemo(int reportType, int reportLevel, const QString &memo)
{
    if (!m_db.isOpen()) { m_lastError = "DB not open"; return false; }
    QVector<QVariantMap> rows = select(
        "SELECT IX FROM T_Report_Template WHERE Report_Type = ? AND Report_LEVEL = ? LIMIT 1",
        { reportType, reportLevel });
    if (rows.isEmpty()) {
        return exec("INSERT INTO T_Report_Template (Report_Type, Report_LEVEL, MEMO) VALUES (?, ?, ?)",
                   { reportType, reportLevel, memo });
    }
    int ix = rows.at(0).value("IX").toInt();
    return exec("UPDATE T_Report_Template SET MEMO = ? WHERE IX = ?", { memo, ix });
}

QVector<QVariantMap> AppDb::getOfferingsTemplateList()
{
    exec("CREATE TABLE IF NOT EXISTS T_Offerings_Template (IX INTEGER PRIMARY KEY AUTOINCREMENT, Offering_Name TEXT, Offering_PhotoPath TEXT, Offering_Price TEXT, Offering_Usage TEXT)");
    QVector<QVariantMap> rows = select("SELECT IX, Offering_Name, Offering_PhotoPath, Offering_Price, Offering_Usage FROM T_Offerings_Template ORDER BY IX ASC");
    for (QVariantMap &m : rows) {
        if (m.contains("Offering_Name")) { m["name"] = m.take("Offering_Name"); }
        if (m.contains("Offering_PhotoPath")) { m["photoPath"] = m.take("Offering_PhotoPath"); }
        if (m.contains("Offering_Price")) { m["price"] = m.take("Offering_Price"); }
        if (m.contains("Offering_Usage")) { m["usage"] = m.take("Offering_Usage"); }
    }
    return rows;
}

int AppDb::insertOfferingsTemplate(const QString &name, const QString &photoPath, const QString &price, const QString &usage)
{
    if (!m_db.isOpen()) { m_lastError = "DB not open"; return -1; }
    QSqlQuery q(m_db);
    if (!q.prepare("INSERT INTO T_Offerings_Template (Offering_Name, Offering_PhotoPath, Offering_Price, Offering_Usage) VALUES (?, ?, ?, ?)")) return -1;
    q.addBindValue(name);
    q.addBindValue(photoPath);
    q.addBindValue(price);
    q.addBindValue(usage);
    if (!q.exec()) { m_lastError = q.lastError().text(); return -1; }
    return q.lastInsertId().toInt();
}

bool AppDb::updateOfferingsTemplate(int ix, const QString &name, const QString &photoPath, const QString &price, const QString &usage)
{
    return exec("UPDATE T_Offerings_Template SET Offering_Name = ?, Offering_PhotoPath = ?, Offering_Price = ?, Offering_Usage = ? WHERE IX = ?",
                { name, photoPath, price, usage, ix });
}

bool AppDb::deleteOfferingsTemplate(int ix)
{
    return exec("DELETE FROM T_Offerings_Template WHERE IX = ?", { ix });
}

int AppDb::getReportTemplateIx(int reportType, int reportLevel)
{
    QVector<QVariantMap> rows = select(
        "SELECT IX FROM T_Report_Template WHERE Report_Type = ? AND Report_LEVEL = ? LIMIT 1",
        { reportType, reportLevel });
    if (!rows.isEmpty())
        return rows.at(0).value("IX").toInt();
    if (!m_db.isOpen()) { m_lastError = "DB not open"; return -1; }
    QSqlQuery q(m_db);
    if (!q.prepare("INSERT INTO T_Report_Template (Report_Type, Report_LEVEL, MEMO) VALUES (?, ?, '')")) return -1;
    q.addBindValue(reportType);
    q.addBindValue(reportLevel);
    if (!q.exec()) { m_lastError = q.lastError().text(); return -1; }
    return q.lastInsertId().toInt();
}

QVector<int> AppDb::getOfferingIxsForReportIx(int reportIx)
{
    QVector<int> out;
    QVector<QVariantMap> rows = select("SELECT Offering_IX FROM T_Report_Offerings_Template WHERE Report_IX = ? ORDER BY IX ASC", { reportIx });
    for (const QVariantMap &m : rows)
        out.append(m.value("Offering_IX").toInt());
    return out;
}

bool AppDb::setOfferingsForReportIx(int reportIx, const QVector<int> &offeringIxs)
{
    if (!exec("DELETE FROM T_Report_Offerings_Template WHERE Report_IX = ?", { reportIx }))
        return false;
    for (int oix : offeringIxs) {
        if (oix <= 0) continue;
        if (!exec("INSERT INTO T_Report_Offerings_Template (Report_IX, Offering_IX) VALUES (?, ?)", { reportIx, oix }))
            return false;
    }
    return true;
}
