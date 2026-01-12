#include "CustomerManager.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QUrl>
#include <QImage>

CustomerManager::CustomerManager(QObject *parent)
    : QObject{parent}
{

}


bool CustomerManager::search(const QString& keyword)
{
    bool ret = false;
    QVector<Customer> finded;
    ret = AppDb::instance().FindCustomers(keyword,finded);
    if(ret)
    {
        customer_.clear();

        for(Customer customer : finded)
        {
            QVariantMap m;
            m["ix"] = customer.IX;
            m["id"] = customer.Cust_ID;
            m["photo"] = customer.Cust_Photo;
            m["name"] = customer.Cust_Name;
            m["date"] = customer.Cust_EditTime;
            m["gender"] = customer.Cust_Gender;
            m["birthday"] = customer.Cust_Birthday;
            m["email"] = customer.Cust_EMail;
            m["phone"] = customer.Cust_Phone;
            customer_.push_back(m);
        }
        emit customersChanged();
    }

    return ret;
}

bool CustomerManager::deleteCustomer(const QString& customerID)
{
    bool ret = false;
    QVector<Customer> finded;
    ret = AppDb::instance().DeleteCustomer(customerID);

    return ret;
}

QString CustomerManager::saveCustomer(const QVariantMap& customer) {
    // 这里你可以先 INSERT DB（略），成功后再刷新
    // 为了简单：直接加到内存（真实项目建议写 DB 后 loadAll）
    Customer c;

    c.IX = customer.value("ix", -1).toInt();
    c.Cust_ID = customer["id"].toString() ;
    c.Cust_Photo = customer["photo"].toString() ;
    c.Cust_Name= customer["name"].toString() ;
    c.Cust_EditTime= customer["date"].toString() ;
    c.Cust_Gender = customer["gender"].toInt() ;
    c.Cust_Birthday =customer["birthday"].toString() ;
    c.Cust_EMail = customer["email"].toString() ;
    c.Cust_Phone = customer["phone"].toString() ;

    bool ret = false;
    if(c.IX==-1)
    { //add.
        ret = AppDb::instance().AddCustomer(c);
        // if(ret)
        // {
        //     customer["ix"] = c.IX;
        //     customer["id"] = c.Cust_ID;
        //     customer_.push_front(customer);
        //     emit customersChanged();
        // }
    }
    else
    {
        ret = AppDb::instance().editCustomer(c);
        // if(ret)
        // {
        //     int n = find_customer_index(c.IX);
        //     customer_[n] = customer;
        //     emit customersChanged();
        // }
    }

    if(customer.value("photo_update",false).toBool() && !c.Cust_Photo.isEmpty())
    {
        QString photo = saveCustomerPhoto(c.Cust_Photo,c.Cust_ID);
        AppDb::instance().updateCustomerPhoto(c.IX,photo);
    }

    return c.Cust_ID;
}

QString CustomerManager::saveCustomerPhoto(const QString &sourcePath, const QString &customerId)
{
    if (sourcePath.isEmpty() || customerId.isEmpty()) return "";

    // 1. 转换路径格式
    QString localSrc = QUrl(sourcePath).toLocalFile();
    if (localSrc.isEmpty()) localSrc = sourcePath; // 处理已经是本地路径的情况

    // 2. 加载图片
    QImage img(localSrc);
    if (img.isNull()) {
        qWarning() << "Failed to load image:" << localSrc;
        return "";
    }

    // 3. 准备目标目录
    QString relativeDir = QString("customers/%1").arg(customerId);
    QString targetDir = QCoreApplication::applicationDirPath() + "/" + relativeDir;
    QDir().mkpath(targetDir);

    // 4. 图片缩放处理
    // 假设我们希望最大宽度或高度不超过 150 像素
    int maxSize = 200;
    if (img.width() > maxSize || img.height() > maxSize) {
        img = img.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // 5. 保存为 JPG 格式（压缩率 90，平衡画质和体积）
    QString targetFileName = "photo.jpg";
    QString fullTargetPath = targetDir + "/" + targetFileName;

    // 保存图片
    if (img.save(fullTargetPath, "JPG", 90)) {
        return relativeDir + "/" + targetFileName;
    }

    return "";
}
int CustomerManager::find_customer_index(const int &ix)
{
    for(int i=0;i<customer_.size();i++)
    {
        const QVariantMap &customer = customer_[i].toMap();
        if(customer["ix"].toInt()==ix)
            return i;
    }
    return -1;
}


