#ifndef FACECONTOURMATH_H
#define FACECONTOURMATH_H

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointF>
#include <QSize>
#include <QVector>

namespace FaceContourMath {

constexpr const char *kCoordPixel = "pixel";
constexpr const char *kCoordLogical768 = "logical768"; // 仅 LibFA 输出临时换算 / 读旧数据
constexpr const char *kScopeGroup = "group";

QPointF logicalToPixel(int logicalX, int logicalY, int imageWidth, int imageHeight);
QPointF pixelToLogical(qreal pixelX, qreal pixelY, int imageWidth, int imageHeight);

QVector<QPointF> logicalJsonToPixels(const QJsonArray &points, int imageWidth, int imageHeight);
QJsonArray pixelsToLogicalJson(const QVector<QPointF> &points, int imageWidth, int imageHeight);

/// LibFA T_CONTOUR logical768 → 原图像素（与 LibFA64_demo 贴图公式一致）
QVector<QPointF> libfaContourToPixels(int pointCount, const int *logicalX, const int *logicalY,
                                      int imageWidth, int imageHeight);

QJsonArray libfaContourToPixelJson(int pointCount, const int *logicalX, const int *logicalY,
                                   int imageWidth, int imageHeight);

QVector<QPointF> jsonToPixels(const QJsonArray &points);
QJsonArray pixelsToJson(const QVector<QPointF> &points);

/// DB 模板表：768×1024 左上角 → 锚点 RGB 真实像素（一次性缩放，不再存 logical768）
QVector<QPointF> templateTopLeft1024JsonToPixels(const QJsonArray &points, int imageWidth, int imageHeight);

bool isGroupSmoothCurve(const QJsonObject &obj);
bool isSmoothCurve(const QJsonObject &obj);

/// auto 轮廓像素跨度是否可信（过窄则视为旧版错误数据，需重做）
bool autoContourPixelSpanPlausible(const QJsonObject &obj, int minSpanPermille = 200);

QJsonObject makeGroupContourJson(const QString &source,
                                 const QJsonArray &pixelPoints,
                                 const QColor &color = Qt::red,
                                 int weight = 3);

} // namespace FaceContourMath

#endif
