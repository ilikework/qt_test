#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

// ImageEditor.h
#include <QtQml/qqmlregistration.h> // 必须包含
#include <QQuickPaintedItem>
#include <QImage>
#include <QPainter>
#include "BaseDrawingItem.h"
#include <memory> // for std::unique_ptr smart point
#include <vector>

// 定义图形对象

class ImageEditor : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QColor penColor MEMBER m_penColor)
    Q_PROPERTY(int penWidth MEMBER m_penWidth)
    Q_PROPERTY(bool shapesVisible READ shapesVisible WRITE setShapesVisible NOTIFY shapesVisibleChanged)

public:
    explicit ImageEditor(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent), m_penColor(Qt::red), m_penWidth(2) {
        setAcceptTouchEvents(true);
        setAcceptedMouseButtons(Qt::LeftButton);
    }

    // 检测是否点中了某个 Item 的控制点，返回包含 itemIx 和 pointIndex 的 Map
    Q_INVOKABLE QVariantMap hitTestPoint(qreal x, qreal y);
    // 实时更新点坐标
    Q_INVOKABLE void updatePoint(int itemIx, int pointIdx, qreal x, qreal y);
    // 设置某个 Item 的编辑状态（是否变黄显示方块）
    Q_INVOKABLE void setEditState(int itemIx, bool editing);
    // 清除所有高亮状态
    Q_INVOKABLE void clearAllEditStates();
    Q_INVOKABLE void setAllItemsEditMode(bool enabled);

    Q_INVOKABLE void init(const int IX, const QString &dirType);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void addLine(qreal x1, qreal y1, qreal x2, qreal y2);
    // 在 ImageEditor 类中添加
    Q_INVOKABLE void addCircle(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3);

    Q_INVOKABLE void eraseAt(qreal x, qreal y);


    Q_INVOKABLE bool save(const QString &path);

    // 设置图片源时初始化图层
    void setSource(const QString &path);

    void paint(QPainter *painter) override;

    QString source() const { return m_sourcePath; }

    bool shapesVisible() {return m_shapesVisible;}
    void setShapesVisible(bool visible) {
        if (m_shapesVisible != visible) {
            m_shapesVisible = visible;
            emit shapesVisibleChanged();
            update(); // 触发重新绘制
        }
    }
signals:
    void sourceChanged();
    void shapesVisibleChanged();

private:
    void loadFromDb(int facePhotoIx, const QString &dirType);
    std::unique_ptr<BaseDrawingItem> createItemFromJson(const QJsonObject &obj, bool scaleFromStandard);

    QImage m_image;
    QString m_sourcePath;
    QColor m_penColor;
    int m_penWidth;
    int m_facePhotoIx = -1;
    bool m_shapesVisible = true;
    std::vector<std::unique_ptr<BaseDrawingItem>> m_items; // 存储所有画上去的对象
};

#endif // IMAGEEDITOR_H
