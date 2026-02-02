#include "BaseDrawingItem.h"


bool LineItem::isHit(const QPointF &pos, qreal threshold) {
    // 使用点到线段的距离公式
    QLineF line(p1, p2);
    qreal l2 = line.length() * line.length();
    if (l2 == 0.0) return QLineF(pos, p1).length() < threshold;

    // 投影比例
    qreal t = ((pos.x() - p1.x()) * (p2.x() - p1.x()) + (pos.y() - p1.y()) * (p2.y() - p1.y())) / l2;
    t = qBound(0.0, t, 1.0); // 限制在线段范围内

    QPointF projection = p1 + t * (p2 - p1);
    return QLineF(pos, projection).length() < threshold;
}

void SmoothCurveItem::paint(QPainter *painter) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path = generateSmoothPath();

    // 逻辑：如果正在被拖动(isBeingEdited)，线条变黄；否则用原始颜色
    QColor lineColor =  this->color;

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(lineColor, width));
    painter->drawPath(path);

    // 逻辑：只要在 EditSmooth 模式下(showControlPoints)，就画方块
    if (showControlPoints) {
        painter->setBrush(Qt::yellow);
        painter->setPen(QPen(Qt::black, 1));
        qreal side = 8.0;
        for (const QPointF &p : points) {
            painter->drawRect(QRectF(p.x() - side/2, p.y() - side/2, side, side));
        }
    }
    painter->restore();
}
