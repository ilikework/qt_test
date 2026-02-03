#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QVector>

#define ISO          "iso"
#define EXPOSURETIME "exposuretime"
#define APERTURE     "aperture"
#define WB           "wb"
#define IMAGESIZE    "ImageSize"
#define IMAGEQUALITY "ImageQuality"

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

class CaptureParameter
{
public:
    int iso = 0;
    QString iso_text = "";
    int exposureTime = 0;
    QString exposureTime_text = "";
    int aperture = 0;
    QString aperture_text = "";
    int wb = 0;
    QString wb_text = "";
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

class FacePhoto
{
public:
    int IX;
    QString Cust_ID;
    QString Photo_CapType;  //RGB/UV/PL/NPL/GRAY/RED/BROWN/WHOLE
    QString Photo_DirType;  //L/R/M
    int     Group_ID;
    int     Photo_ID;
    QString Photo_Name;
    QString Photo_EditTime;
    QString photo_iso;
    QString photo_wb;
    QString photo_aperture;
    QString photo_exposuretime;
    QString photo_comment;
};

class AppDb : public QObject {
    Q_OBJECT
public:
    static AppDb& instance();
    bool openDb();
    void closeDb();

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
    bool addCustomer(Customer &customer); // need return IX and ID
    bool DeleteCustomer(const QString &CustomID);
    bool editCustomer(const Customer &customer);
    bool updateCustomerPhoto(const int IX, const QString &photo);
    bool addPhoto(FacePhoto &photo);
    bool findPhotoesbyCustomID(const QString &CustomID, QVector<FacePhoto> &photoList);
    bool findPhotoesbyCustomIDandGropuID(const QString &CustomID, const int nGroupID, QVector<FacePhoto> &photoList);

    // 插入单条绘图信息，并返回该条记录的 IX (主键)
    int insertDrawInfo(int facePhotoIx, const QString& jsonInfo);

    // 根据主键 IX 物理删除单条记录
    bool deleteDrawInfoByIX(int ix);

    // 一次性获取某张图片关联的所有绘图信息
    // 返回值：QList<QPair<主键IX, JSON字符串>>
    QList<QPair<int, QString>> getAllDrawInfos(int facePhotoIx);
    QString getTemplateInfo(const QString &dirType);

    // 预录：T_Report_Template 建议文字（Report_Type 1-8,100；Report_LEVEL 10=差,20=中,30=好）
    QString getReportTemplateMemo(int reportType, int reportLevel);
    bool setReportTemplateMemo(int reportType, int reportLevel, const QString &memo);

    // 预录：T_Offerings_Template 产品/服务目录
    QVector<QVariantMap> getOfferingsTemplateList();
    int insertOfferingsTemplate(const QString &name, const QString &photoPath, const QString &price, const QString &usage);
    bool updateOfferingsTemplate(int ix, const QString &name, const QString &photoPath, const QString &price, const QString &usage);
    bool deleteOfferingsTemplate(int ix);

    // 预录：T_Report_Offerings_Template，Report_IX -> 多个 Offering_IX（一个报告模板可关联多个产品/服务）
    int getReportTemplateIx(int reportType, int reportLevel); // 获取或创建 T_Report_Template 行，返回 IX
    QVector<int> getOfferingIxsForReportIx(int reportIx);
    bool setOfferingsForReportIx(int reportIx, const QVector<int> &offeringIxs);

private:
    explicit AppDb(QObject* parent=nullptr);
    ~AppDb();

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
