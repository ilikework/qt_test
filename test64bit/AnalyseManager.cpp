#include "AnalyseManager.h"
#include "AppDb.h"
#include <QCoreApplication>
#include <QUrl>
#include "MM_Const_Define.h"

AnalyseManager::AnalyseManager(QObject *parent)
    : QObject{parent}
{}

QString AnalyseManager::get_folder(int nGroupID) const
{
    return QCoreApplication::applicationDirPath()
            + SLASH + DIR_CUSTOMERS
            + SLASH +CustomerID_
            + SLASH +QString("%1").arg(nGroupID, 2, 10, QChar('0'));
}

void AnalyseManager::init(const QString & strCustomerID)
{
    listThumb_.clear();

    CustomerID_ = strCustomerID;
    QVector<FacePhoto> photolist;
    AppDb::instance().findPhotoesbyCustomID(strCustomerID,photolist);

    // 临时结构，用于配对每一组的照片
    // QMap<GroupID, QMap<PhotoID, QVariantMap>>
    //QMap<int, QMap<int, QVariantMap>> groupGroups;

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
                    {

                        item["photoL"] = QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" +  var.Photo_Name);
                        item["IXL"] = var.IX;
                    }
                    else if(var.Photo_DirType==RIGHT)
                    {
                        item["photoR"] =  QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" + var.Photo_Name);
                        item["IXR"] = var.IX;
                    }
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
                {
                    newItem["photoL"] = QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" +  var.Photo_Name);
                    newItem["IXL"] = var.IX;
                }
                else if(var.Photo_DirType==RIGHT)
                {
                    newItem["photoR"] =  QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" + var.Photo_Name);
                    newItem["IXR"] = var.IX;
                }
                listThumb_.append(newItem);
            }
        }

        // --- 逻辑 2: 填充详情映射 (所有的照片) ---
        // 填充详情映射数据
        // if (var.Photo_DirType==LEFT) {
        //     groupGroups[var.Group_ID][var.Photo_ID]["photoL"] = QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" +  var.Photo_Name);
        // } else {
        //     groupGroups[var.Group_ID][var.Photo_ID]["photoR"] = QUrl::fromLocalFile(get_folder(var.Group_ID)+ "/" +  var.Photo_Name);
        // }

    }

    // // 将 groupGroups 转换为详情 Map (键为字符串)
    // for (auto itG = groupGroups.begin(); itG != groupGroups.end(); ++itG) {
    //     QVariantList subList;
    //     for (auto itP = itG.value().begin(); itP != itG.value().end(); ++itP) {
    //         subList.append(itP.value()); // 加入包含 LEFT 和 RIGHT 的对象
    //     }
    //     mapDetail_[QString::number(itG.key())] = subList;
    // }

     emit thumbphotoesChanged();
}

QVariantList AnalyseManager::loadSub(const int & nGroupID)
{
    QVector<FacePhoto> photolist;
    AppDb::instance().findPhotoesbyCustomIDandGropuID(CustomerID_,nGroupID,photolist);

    // 使用 Map 按 Photo_ID 暂存数据，Key 是 Photo_ID
    // Value 是一个包含 L 和 R 信息的 Map
    QMap<int, QVariantMap> pairMap;

    for (const auto& photo : photolist) {
        int id = photo.Photo_ID;

        // 如果这个 ID 还没在 Map 里，初始化它
        if (!pairMap.contains(id)) {
            pairMap[id] = QVariantMap();
        }

        // 根据方向填入对应的字段
        if (photo.Photo_DirType == LEFT) {
            pairMap[id]["photoL"] =  QUrl::fromLocalFile(get_folder(photo.Group_ID)+ "/" +  photo.Photo_Name);
            pairMap[id]["IXL"] = photo.IX;
        } else if (photo.Photo_DirType == RIGHT) {
            pairMap[id]["photoR"] =  QUrl::fromLocalFile(get_folder(photo.Group_ID)+ "/" +  photo.Photo_Name);
            pairMap[id]["IXR"] = photo.IX;
        }
    }

    // 将 Map 转换为 QVariantList 返回给 QML
    QVariantList result;
    // 使用 Map 的 keys 确保输出顺序（通常按 Photo_ID 从小到大）
    for (int id : pairMap.keys()) {
        result.append(pairMap[id]);
    }

    return result;
}

QVariantList AnalyseManager::thumbphotoes() const
{
    return listThumb_;
}
