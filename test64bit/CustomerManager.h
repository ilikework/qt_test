#ifndef CUSTOMERMANAGER_H
#define CUSTOMERMANAGER_H

#include <QObject>
#include <QVariantList>

class CustomerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList customers READ customers NOTIFY customersChanged)
public:
    explicit CustomerManager(QObject *parent = nullptr);

    QVariantList  customers() const {return customer_;}
    Q_INVOKABLE bool search(const QString& keyword);
    Q_INVOKABLE QString saveCustomer(const QVariantMap& customer);
    Q_INVOKABLE bool deleteCustomer(const QString& customerID);
    int find_customer_index(const int &ix);
signals:
    void customersChanged();


private:
    QVariantList  customer_;
    QString saveCustomerPhoto(const QString &sourcePath, const QString &customerId);
    static QString getStr(const QVariantMap& m, const char* key);
};

#endif // CUSTOMERMANAGER_H
