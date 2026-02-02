#ifndef ANALYSEMANAGER_H
#define ANALYSEMANAGER_H

#include <QObject>
#include <QVariantList>

class AnalyseManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList thumbphotoes READ thumbphotoes NOTIFY thumbphotoesChanged)
public:
    explicit AnalyseManager(QObject *parent = nullptr);

    QVariantList thumbphotoes() const;

    Q_INVOKABLE void init(const QString & strCustomerID);
    Q_INVOKABLE QVariantList loadSub(const int & nGroupID);


signals:
    void thumbphotoesChanged();

private:
    QString get_folder(int nGroupID) const;

    QString CustomerID_ = "";
    int     nCurGroupID_ = 0;
    int     nCurPhotoID_ = 0;

    QVariantList listThumb_;
    //QVariantMap  mapDetail_;

};

#endif // ANALYSEMANAGER_H
