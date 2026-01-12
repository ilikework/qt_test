#ifndef ANALYSEMANAGER_H
#define ANALYSEMANAGER_H

#include <QObject>
#include <QVariantList>

class AnalyseManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList tmbphotoes READ tmbphotoes NOTIFY tmbphotoesChanged)
public:
    explicit AnalyseManager(QObject *parent = nullptr);

    QVariantList tmbphotoes() const;

    Q_INVOKABLE void init(const QString & strCustomerID);


signals:
    void tmbphotoesChanged();

private:
    QString get_folder(int nGroupID) const;

    QString CustomerID_ = "";
    int     nCurGroupID_ = 0;
    int     nCurPhotoID_ = 0;

    QVariantList listThumb_;
    QVariantMap  mapDetail_;

};

#endif // ANALYSEMANAGER_H
