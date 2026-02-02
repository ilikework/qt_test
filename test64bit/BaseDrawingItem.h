#ifndef BASEDRAWINGITEM_H
#define BASEDRAWINGITEM_H

#include <QPoint>
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QPainterPath>
#include <QPen>
#include <QVector>

#define VER1 "V1"

// 基础图形类
class BaseDrawingItem {
public:
    virtual ~BaseDrawingItem() = default;
    virtual void paint(QPainter *painter) = 0;              // 核心绘制逻辑
    virtual bool isHit(const QPointF &pos, qreal threshold) = 0; // 橡皮擦碰撞检测
    //virtual void save() = 0; // 数据库保存
    virtual QJsonObject toJson() const = 0;

    int ix = -1; // 对应数据库 T_FacePhoto_DrawInfo 的 IX，-1 表示新创建尚未同步
    QColor color;
    int width;

    bool isBeingEdited =false;
};

// 线段类
class LineItem : public BaseDrawingItem {
public:
    QPointF p1, p2;

    LineItem(QPointF a, QPointF b, QColor c, int w) : p1(a), p2(b) { color = c; width = w; }
    void paint(QPainter *painter) override {
        painter->setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(p1, p2);
    }
    bool isHit(const QPointF &pos, qreal threshold) override;
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["version"] = VER1;
        obj["type"] = "line";
        obj["weight"] = width;
        obj["color"] = color.name();
        QJsonArray pts;
        pts.append(QJsonObject{{"x", p1.x()}, {"y", p1.y()}});
        pts.append(QJsonObject{{"x", p2.x()}, {"y", p2.y()}});
        obj["points"] = pts;
        return obj;
    }
    //void save() override;
};

// 圆形类
class CircleItem : public BaseDrawingItem {
public:
    QPointF p1, p2, p3; // 记录原始三点，方便以后导出数据
    QPointF center;
    qreal radius;

    CircleItem(QPointF p1, QPointF p2,QPointF p3, QColor c, int w) : p1(p1), p2(p2), p3(p3){
        color = c; width = w;
        calculateGeometry(); // 构造时计算圆心和半径
    }

    void paint(QPainter *painter) override {
        painter->setPen(QPen(color, width));
        painter->drawEllipse(center, radius, radius);
    }

    bool isHit(const QPointF &pos, qreal threshold) override {
        // 判断点击位置到圆心的距离是否接近半径
        qreal dist = QLineF(pos, center).length();
        return qAbs(dist - radius) < threshold;
    }

    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["version"] = VER1;
        obj["type"] = "circle";
        obj["weight"] = width;
        obj["color"] = color.name();
        QJsonArray pts; // 存储原始三点坐标，以便精确还原
        pts.append(QJsonObject{{"x", p1.x()}, {"y", p1.y()}});
        pts.append(QJsonObject{{"x", p2.x()}, {"y", p2.y()}});
        pts.append(QJsonObject{{"x", p3.x()}, {"y", p3.y()}});
        obj["points"] = pts;
        obj["center"] = QJsonObject{{"x", center.x()}, {"y", center.y()}};
        obj["radius"] = radius;
        return obj;
    }

private:
    void calculateGeometry() {
        // 将你原来的三点求圆算法移到这里
        qreal x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y(), x3 = p3.x(), y3 = p3.y();
        qreal x12 = x1 - x2; qreal x13 = x1 - x3;
        qreal y12 = y1 - y2; qreal y13 = y1 - y3;
        qreal y31 = y3 - y1; qreal y21 = y2 - y1;
        qreal x31 = x3 - x1; qreal x21 = x2 - x1;
        qreal sx13 = pow(x1, 2) - pow(x3, 2); qreal sy13 = pow(y1, 2) - pow(y3, 2);
        qreal sx21 = pow(x2, 2) - pow(x1, 2); qreal sy21 = pow(y2, 2) - pow(y1, 2);
        qreal f = ((sx13) * (x12) + (sy13) * (x12) + (sx21) * (x13) + (sy21) * (x13)) / (2 * (y31 * x12 - y21 * x13));
        qreal g = ((sx13) * (y12) + (sy13) * (y12) + (sx21) * (y13) + (sy21) * (y13)) / (2 * (x31 * y12 - x21 * y13));
        center = QPointF(-g, -f);
        radius = sqrt(pow(center.x() - x1, 2) + pow(center.y() - y1, 2));
    }
};


class SmoothCurveItem : public BaseDrawingItem {
public:
    QVector<QPointF> points; // 已经转换好的、直接对应图片像素的坐标
    bool isClosed;
    bool showControlPoints= false;

    // 构造函数 A：直接加载（用于从 T_FacePhoto_DrawInfo 读取已经转换好的点）
    SmoothCurveItem(const QVector<QPointF> &pts, QColor c, int w, bool closed = true)
        : isClosed(closed)
    {
        this->points = pts;
        this->color = c;
        this->width = w;
    }

    /**
     * @param rawPts    从数据库读取的原始坐标 (768x1024 尺度)
     * @param imgSize   当前关联图片的实际像素尺寸 (例如 3000x4000)
     * @param c         颜色
     * @param w         线宽
     * @param closed    是否封闭
     */
    SmoothCurveItem(const QVector<QPointF> &rawPts, const QSize &imgSize, QColor c, int w, bool closed = true)
        : isClosed(closed)
    {
        this->color = c;
        this->width = w;

        // 1. 在构造函数中直接完成坐标转换
        qreal sx = imgSize.width() / 768.0;
        qreal sy = imgSize.height() / 1024.0;

        for (const QPointF &p : rawPts) {
            this->points << QPointF(p.x() * sx, p.y() * sy);
        }
    }

    void paint(QPainter *painter) override;

    // 3. 碰撞检测：同样基于实际像素坐标，逻辑更直观
    bool isHit(const QPointF &pos, qreal threshold) override {
        QPainterPath path = generateSmoothPath();
        if (isClosed && path.contains(pos)) return true;

        QPainterPathStroker stroker;
        stroker.setWidth(width + threshold);
        return stroker.createStroke(path).contains(pos);
    }

    /**
 * @brief 检测鼠标是否落在了某个具体的控制点上
 * @param pos        鼠标点击的坐标
 * @param threshold  检测半径（点击容错范围）
 * @return int       返回点中的点的索引 (0 ~ N-1)，如果没有点中则返回 -1
 */
    int hitTestPoint(const QPointF &pos, qreal threshold) {
        for (int i = 0; i < points.size(); ++i) {
            // 计算点击位置与当前点的距离
            // 使用 QVector2D 或者简单的距离公式
            QPointF delta = pos - points[i];
            qreal distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

            if (distance <= threshold) {
                return i; // 找到了点中的点，返回其索引
            }
        }
        return -1; // 没有任何点被选中
    }

    // 4. 保存：导出当前对应图片的实际坐标
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["type"] = "smooth_curve";
        obj["version"] = VER1;
        obj["color"] = color.name();
        obj["weight"] = width;

        QJsonArray ptsArray;
        for (const QPointF &p : points) {
            QJsonObject pObj;
            pObj["x"] = p.x();
            pObj["y"] = p.y();
            ptsArray.append(pObj);
        }
        obj["points"] = ptsArray;
        return obj;
    }

private:
    // 内部路径生成（Catmull-Rom）
    QPainterPath generateSmoothPath() const {
        QPainterPath path;
        int n = points.size();
        if (n < 2) return path;

        // 闭合曲线的辅助点处理
        QVector<QPointF> p;
        if (isClosed) {
            p << points[n - 1] << points << points[0] << points[1];
        } else {
            p << points[0] << points << points[n - 1];
        }

        path.moveTo(p[1]);
        for (int i = 1; i < p.size() - 2; ++i) {
            QPointF p1 = p[i];
            QPointF p2 = p[i + 1];
            QPointF cp1 = p1 + (p[i + 1] - p[i - 1]) / 6.0;
            QPointF cp2 = p2 - (p[i + 2] - p[i]) / 6.0;
            path.cubicTo(cp1, cp2, p2);
        }

        if (isClosed) path.closeSubpath();
        return path;
    }

};
#endif // BASEDRAWINGITEM_H
