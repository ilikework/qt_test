#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QVector>

struct CaptureSetting
{
    int series = 0;

    int imageSize = 0;
    int imageQuality = 0;

    int rgb_iso = 0;
    int rgb_exposureTime = 0;
    int rgb_aperture = 0;
    int rgb_wb = 0;

    int uv_iso = 0;
    int uv_exposureTime = 0;
    int uv_aperture = 0;
    int uv_wb = 0;

    int pl_iso = 0;
    int pl_exposureTime = 0;
    int pl_aperture = 0;
    int pl_wb = 0;

    int npl_iso = 0;
    int npl_exposureTime = 0;
    int npl_aperture = 0;
    int npl_wb = 0;

    int gender = 0;
};

class Customer
{
public:
    int     IX;
    QString Cust_ID;
    QString Cust_Name;
    int     Cust_Gender;
    QString Cust_Birthday; // yyyy-MM-dd
    QString Cust_EditTime; // yyyy-MM-dd
    QString Cust_Phone;
    QString Cust_Addr;
    QString Cust_EMail;
    QString Cust_Photo;
    //QString Cust_Height;
    //QString Cust_Weight;
    QString Cust_Des;
};

class AppDb : public QObject {
    Q_OBJECT
public:
    static AppDb& instance();

    bool openSqlite(const QString& filePath);
    bool isOpen() const;
    QString lastErrorText() const;

    // 通用查询：返回 rows（每行是 QVariantMap：列名->值）
    QVector<QVariantMap> select(const QString& sql, const QVariantList& binds = {});



    QVariantList cameraConfigList(const int nCameraSeries, const QString& configType);
    bool loadCaptureSettingBySeries(const int nCameraSeries, CaptureSetting& outSetting);
    void CaptureSettingUpdate(const int nCameraSeries, const QString& column, const int value);
    int GetNextGroupID(const QString &CustomID);
    bool FindCustomers(const QString &filter, QVector<Customer> &customerList);
    QString GetNextCustomID();
    bool AddCustomer(Customer &customer); // need return IX and ID
    bool DeleteCustomer(const QString &CustomID);
    bool editCustomer(const Customer &customer);
    bool updateCustomerPhoto(const int IX, const QString &photo);


private:
    explicit AppDb(QObject* parent=nullptr);
    ~AppDb();

    bool openDb();
    // 通用执行：insert/update/delete
    bool exec(const QString& sql, const QVariantList& binds = {});
    // 事务（可选）
    bool begin();
    bool commit();
    bool rollback();

    QSqlDatabase m_db;
    QString m_connName;
    QString m_lastError;

};
