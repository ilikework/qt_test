#include "AnalyseManager.h"
#include "AppDb.h"
#include <QCoreApplication>
#include "MM_Const_Define.h"

AnalyseManager::AnalyseManager(QObject *parent)
    : QObject{parent}
{}

QString AnalyseManager::get_folder(int nGroupID) const
{
    return QCoreApplication::applicationDirPath()
            + DIR_CUSTOMERS
            + "/" +CustomerID_
            + "/" +QString("%1").arg(nGroupID, 2, 10, QChar('0'));
}

void AnalyseManager::init(const QString & strCustomerID)
{
    listThumb_.clear();

    CustomerID_ = strCustomerID;
    QVector<FacePhoto> photolist;
    AppDb::instance().findPhotoesbyCustomID(strCustomerID,photolist);

    for (const FacePhoto& var: photolist) {
        QVariantMap m;
        QVariantMap m2;
        if(var.Photo_ID==1)
        {
            // 查找 thumbPhotoes_ 中是否已经存在该 GroupID
            bool found = false;
            for (int i = 0; i < listThumb_.size(); ++i)
            {
                QVariantMap item = listThumb_[i].toMap();
                if (item["GROUPID"].toInt() == var.Group_ID)
                {
                    if (var.Photo_DirType==LEFT)
                        item["photoL"] = get_folder(var.Group_ID)+ "/" +  var.Photo_Name;
                    else if(var.Photo_DirType==RIGHT)
                        item["photoR"] =  get_folder(var.Group_ID)+ "/" + var.Photo_Name;
                    listThumb_[i] = item;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                QVariantMap newItem;
                newItem["GROUPID"] = var.Group_ID;
                if (var.Photo_DirType==LEFT)
                    newItem["photoL"] = get_folder(var.Group_ID)+ "/" +  var.Photo_Name;
                else if(var.Photo_DirType==RIGHT)
                    newItem["photoR"] =  get_folder(var.Group_ID)+ "/" + var.Photo_Name;
                listThumb_.append(newItem);
            }
        }

        // --- 逻辑 2: 填充详情映射 (所有的照片) ---
        QString gKey = QString::number(var.Group_ID);
        if (!mapDetail_.contains(gKey)) {
            QVariantMap groupData;
            groupData["photoL"] = QVariantList();
            groupData["photoR"] = QVariantList();
            mapDetail_[gKey] = groupData;
        }

        QVariantMap currentGroup = mapDetail_[gKey].toMap();
        if (var.Photo_DirType==LEFT)
        {
            QVariantList qlist = currentGroup["photoL"].toList();
            qlist.append(var.Photo_Name);
            currentGroup["photoL"] = qlist;
        }
        else
        {
            QVariantList qlist = currentGroup["photoR"].toList();
            qlist.append(var.Photo_Name);
            currentGroup["photoR"] = qlist;
        }
        mapDetail_[gKey] = currentGroup;


    }
}

QVariantList AnalyseManager::tmbphotoes() const
{
    return listThumb_;
}
