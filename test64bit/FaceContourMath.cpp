#include "FaceContourMath.h"

#include <QColor>
#include <QtMath>

namespace FaceContourMath {

QPointF logicalToPixel(int logicalX, int logicalY, int imageWidth, int imageHeight)
{
    if (imageWidth <= 0 || imageHeight <= 0)
        return QPointF();
    const double scale = static_cast<double>(imageWidth) / 768.0;
    const double fullX = logicalX * scale;
    const double fullY = (imageHeight - 1) - logicalY * scale;
    return QPointF(qRound(fullX), qRound(fullY));
}

QPointF pixelToLogical(qreal pixelX, qreal pixelY, int imageWidth, int imageHeight)
{
    if (imageWidth <= 0 || imageHeight <= 0)
        return QPointF();
    const double scale = static_cast<double>(imageWidth) / 768.0;
    if (qFuzzyIsNull(scale))
        return QPointF();
    const double logicalX = pixelX / scale;
    const double logicalY = ((imageHeight - 1) - pixelY) / scale;
    return QPointF(logicalX, logicalY);
}

QVector<QPointF> logicalJsonToPixels(const QJsonArray &points, int imageWidth, int imageHeight)
{
    QVector<QPointF> out;
    out.reserve(points.size());
    for (const QJsonValue &v : points) {
        const QJsonObject p = v.toObject();
        const QPointF px = logicalToPixel(p.value(QStringLiteral("x")).toInt(),
                                         p.value(QStringLiteral("y")).toInt(),
                                         imageWidth, imageHeight);
        out.append(px);
    }
    return out;
}

QJsonArray pixelsToLogicalJson(const QVector<QPointF> &points, int imageWidth, int imageHeight)
{
    QJsonArray arr;
    for (const QPointF &p : points) {
        const QPointF logical = pixelToLogical(p.x(), p.y(), imageWidth, imageHeight);
        QJsonObject o;
        o.insert(QStringLiteral("x"), qRound(logical.x()));
        o.insert(QStringLiteral("y"), qRound(logical.y()));
        arr.append(o);
    }
    return arr;
}

QVector<QPointF> libfaContourToPixels(int pointCount, const int *logicalX, const int *logicalY,
                                      int imageWidth, int imageHeight)
{
    QVector<QPointF> out;
    if (!logicalX || !logicalY || pointCount <= 0 || imageWidth <= 0 || imageHeight <= 0)
        return out;
    const double toFullScale = static_cast<double>(imageWidth) / 768.0;
    out.reserve(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        const int fullX = static_cast<int>(logicalX[i] * toFullScale + 0.5);
        const int fullY = static_cast<int>(imageHeight - 1 - logicalY[i] * toFullScale + 0.5);
        out.append(QPointF(fullX, fullY));
    }
    return out;
}

QJsonArray libfaContourToPixelJson(int pointCount, const int *logicalX, const int *logicalY,
                                   int imageWidth, int imageHeight)
{
    return pixelsToJson(libfaContourToPixels(pointCount, logicalX, logicalY, imageWidth, imageHeight));
}

QVector<QPointF> jsonToPixels(const QJsonArray &points)
{
    QVector<QPointF> out;
    out.reserve(points.size());
    for (const QJsonValue &v : points) {
        const QJsonObject p = v.toObject();
        out.append(QPointF(p.value(QStringLiteral("x")).toDouble(),
                           p.value(QStringLiteral("y")).toDouble()));
    }
    return out;
}

QJsonArray pixelsToJson(const QVector<QPointF> &points)
{
    QJsonArray arr;
    for (const QPointF &p : points) {
        QJsonObject o;
        o.insert(QStringLiteral("x"), qRound(p.x()));
        o.insert(QStringLiteral("y"), qRound(p.y()));
        arr.append(o);
    }
    return arr;
}

QVector<QPointF> templateTopLeft1024JsonToPixels(const QJsonArray &points, int imageWidth, int imageHeight)
{
    QVector<QPointF> out;
    if (imageWidth <= 0 || imageHeight <= 0)
        return out;
    const double sx = static_cast<double>(imageWidth) / 768.0;
    const double sy = static_cast<double>(imageHeight) / 1024.0;
    out.reserve(points.size());
    for (const QJsonValue &v : points) {
        const QJsonObject p = v.toObject();
        out.append(QPointF(qRound(p.value(QStringLiteral("x")).toDouble() * sx),
                           qRound(p.value(QStringLiteral("y")).toDouble() * sy)));
    }
    return out;
}

bool isSmoothCurve(const QJsonObject &obj)
{
    return obj.value(QStringLiteral("type")).toString() == QStringLiteral("smooth_curve");
}

bool autoContourPixelSpanPlausible(const QJsonObject &obj, int minSpanPermille)
{
    if (obj.value(QStringLiteral("source")).toString() != QStringLiteral("auto"))
        return true;
    const QJsonArray pts = obj.value(QStringLiteral("points")).toArray();
    if (pts.size() < 3)
        return false;

    qreal minX = 1e18, maxX = -1e18, minY = 1e18, maxY = -1e18;
    for (const QJsonValue &v : pts) {
        const QJsonObject p = v.toObject();
        const qreal x = p.value(QStringLiteral("x")).toDouble();
        const qreal y = p.value(QStringLiteral("y")).toDouble();
        minX = qMin(minX, x);
        maxX = qMax(maxX, x);
        minY = qMin(minY, y);
        maxY = qMax(maxY, y);
    }
    const qreal spanX = maxX - minX;
    const qreal spanY = maxY - minY;
    const qreal ref = qMax(qMax(maxX, maxY), 1.0);
    const qreal span = qMax(spanX, spanY);
    return span * 1000.0 / ref >= minSpanPermille;
}

bool isGroupSmoothCurve(const QJsonObject &obj)
{
    if (!isSmoothCurve(obj))
        return false;
    const QString scope = obj.value(QStringLiteral("scope")).toString();
    if (scope.isEmpty())
        return true;
    return scope == QLatin1String(kScopeGroup);
}

QJsonObject makeGroupContourJson(const QString &source,
                                 const QJsonArray &pixelPoints,
                                 const QColor &color,
                                 int weight)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QStringLiteral("smooth_curve"));
    obj.insert(QStringLiteral("version"), QStringLiteral("V1"));
    obj.insert(QStringLiteral("scope"), QLatin1String(kScopeGroup));
    obj.insert(QStringLiteral("source"), source);
    obj.insert(QStringLiteral("pointCount"), pixelPoints.size());
    obj.insert(QStringLiteral("color"), color.name());
    obj.insert(QStringLiteral("weight"), weight);
    obj.insert(QStringLiteral("points"), pixelPoints);
    return obj;
}

} // namespace FaceContourMath
