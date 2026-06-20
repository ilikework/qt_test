#include "BaseDrawingItem.h"
#include <QtMath>

namespace {
constexpr qreal kControlPointSide = 8.0;
constexpr qreal kHandleRadius = 9.0;
constexpr qreal kBoundsPadding = 12.0;
constexpr qreal kRotateHandleOffset = 40.0;

bool nearPoint(const QPointF &pos, const QPointF &target, qreal radius)
{
    const QPointF d = pos - target;
    return (d.x() * d.x() + d.y() * d.y()) <= radius * radius;
}
} // namespace

bool LineItem::isHit(const QPointF &pos, qreal threshold) {
    // 沿线段生成稍宽的命中带（圆角矩形/胶囊形），比纯点到线距离更易擦中
    QPainterPath path;
    path.moveTo(p1);
    path.lineTo(p2);

    QPainterPathStroker stroker;
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setWidth(qMax<qreal>(width, 1.0) + threshold * 2.0);
    return stroker.createStroke(path).contains(pos);
}

void SmoothCurveItem::paint(QPainter *painter) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path = generateSmoothPath();

    QColor lineColor = this->color;
    if (isBeingEdited)
        lineColor = QColor("#ffcc00");

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(lineColor, width));
    painter->drawPath(path);

    if (showControlPoints) {
        const QRectF bounds = pointsBounds(kBoundsPadding);

        painter->setPen(QPen(QColor("#66ccff"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(bounds);

        const QPointF rotateHandle(bounds.center().x(), bounds.top() - kRotateHandleOffset);
        const QPointF corners[4] = {
            bounds.topLeft(), bounds.topRight(), bounds.bottomRight(), bounds.bottomLeft()
        };

        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(QColor("#00ccff"));
        for (const QPointF &corner : corners) {
            painter->drawRect(QRectF(corner.x() - kHandleRadius, corner.y() - kHandleRadius,
                                     kHandleRadius * 2, kHandleRadius * 2));
        }

        painter->setBrush(QColor("#66ff99"));
        painter->drawEllipse(rotateHandle, kHandleRadius, kHandleRadius);
        painter->drawLine(bounds.center(), rotateHandle);

        painter->setBrush(Qt::yellow);
        painter->setPen(QPen(Qt::black, 1));
        for (const QPointF &p : points) {
            painter->drawRect(QRectF(p.x() - kControlPointSide / 2, p.y() - kControlPointSide / 2,
                                     kControlPointSide, kControlPointSide));
        }
    }
    painter->restore();
}

QPointF SmoothCurveItem::centroid() const
{
    if (points.isEmpty())
        return QPointF();
    QPointF sum(0, 0);
    for (const QPointF &p : points)
        sum += p;
    return sum / points.size();
}

QRectF SmoothCurveItem::pointsBounds(qreal padding) const
{
    if (points.isEmpty())
        return QRectF();
    qreal minX = points[0].x();
    qreal maxX = minX;
    qreal minY = points[0].y();
    qreal maxY = minY;
    for (const QPointF &p : points) {
        minX = qMin(minX, p.x());
        maxX = qMax(maxX, p.x());
        minY = qMin(minY, p.y());
        maxY = qMax(maxY, p.y());
    }
    QRectF bounds(minX, minY, maxX - minX, maxY - minY);
    bounds.adjust(-padding, -padding, padding, padding);
    return bounds;
}

SmoothCurveItem::SmoothEditHit SmoothCurveItem::hitTestEdit(const QPointF &pos, qreal threshold) const
{
    SmoothEditHit hit;
    const qreal pointRadius = qMax(threshold, kControlPointSide);

    for (int i = 0; i < points.size(); ++i) {
        if (nearPoint(pos, points[i], pointRadius)) {
            hit.type = EditHitPoint;
            hit.pointIndex = i;
            return hit;
        }
    }

    const QRectF bounds = pointsBounds(kBoundsPadding);
    if (bounds.isNull())
        return hit;

    const QPointF rotateHandle(bounds.center().x(), bounds.top() - kRotateHandleOffset);
    if (nearPoint(pos, rotateHandle, kHandleRadius + 4.0)) {
        hit.type = EditHitRotate;
        return hit;
    }

    const QPointF corners[4] = {
        bounds.topLeft(), bounds.topRight(), bounds.bottomRight(), bounds.bottomLeft()
    };
    for (const QPointF &corner : corners) {
        if (nearPoint(pos, corner, kHandleRadius + 4.0)) {
            hit.type = EditHitScale;
            return hit;
        }
    }

    QPainterPath path = generateSmoothPath();
    if ((isClosed && path.contains(pos)) || bounds.contains(pos)) {
        hit.type = EditHitMove;
        return hit;
    }

    return hit;
}

void SmoothCurveItem::translatePoints(const QPointF &delta)
{
    for (QPointF &p : points)
        p += delta;
}

void SmoothCurveItem::scalePointsUniform(qreal factor, const QPointF &origin)
{
    if (factor <= 0.0)
        return;
    for (QPointF &p : points) {
        const QPointF v = p - origin;
        p = origin + v * factor;
    }
}

void SmoothCurveItem::rotatePoints(qreal radians, const QPointF &origin)
{
    const qreal cosA = std::cos(radians);
    const qreal sinA = std::sin(radians);
    for (QPointF &p : points) {
        const QPointF v = p - origin;
        p = origin + QPointF(v.x() * cosA - v.y() * sinA, v.x() * sinA + v.y() * cosA);
    }
}
