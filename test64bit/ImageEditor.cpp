#include "ImageEditor.h"
#include "AppDb.h"
#include "FaceContourMath.h"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
#include "AppDb.h"
#include <QtMath>
#include <QImageReader>

void ImageEditor::init(const int IX, const QString &dirType)
{
    m_facePhotoIx = IX;
    m_dirType = dirType;
    if (!m_image.isNull())
        loadFromDb(IX, dirType);
}

void ImageEditor::reloadDrawings()
{
    if (m_facePhotoIx >= 0)
        loadFromDb(m_facePhotoIx, m_dirType);
}

void ImageEditor::clear()
{
}

bool ImageEditor::save(const QString &path)
{
    QImage saveImg = m_image.copy();
    QPainter p(&saveImg);
    return saveImg.save(QUrl(path).toLocalFile());
}


void ImageEditor::setSource(const QString &path)
{
    m_sourcePath = path;
    QImageReader reader(QUrl(path).toLocalFile());
    reader.setAutoTransform(false);
    if (reader.read(&m_image)) {
        setImplicitWidth(m_image.width());
        setImplicitHeight(m_image.height());
        setWidth(m_image.width());
        setHeight(m_image.height());
        if (m_facePhotoIx >= 0)
            loadFromDb(m_facePhotoIx, m_dirType);
        else
            update();
        emit sourceChanged();
    }
}

qreal ImageEditor::imageToViewScaleX() const
{
    if (m_image.isNull() || m_image.width() <= 0 || width() <= 0)
        return 1.0;
    return width() / static_cast<qreal>(m_image.width());
}

qreal ImageEditor::imageToViewScaleY() const
{
    if (m_image.isNull() || m_image.height() <= 0 || height() <= 0)
        return 1.0;
    return height() / static_cast<qreal>(m_image.height());
}

QPointF ImageEditor::mapViewToImage(qreal x, qreal y) const
{
    const qreal sx = imageToViewScaleX();
    const qreal sy = imageToViewScaleY();
    if (qFuzzyCompare(sx, 1.0) && qFuzzyCompare(sy, 1.0))
        return QPointF(x, y);
    if (qFuzzyCompare(sx, 0.0) || qFuzzyCompare(sy, 0.0))
        return QPointF(x, y);
    return QPointF(x / sx, y / sy);
}

void ImageEditor::addLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    // 不再直接使用 QPainter 画图，而是存入列表
    auto item = std::make_unique<LineItem>(QPointF(x1,y1), QPointF(x2,y2), m_penColor, m_penWidth);

    // 将单个 item 转为 JSON 并存入数据库
    QString jsonString = QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact);
    int newIx = AppDb::instance().insertDrawInfo(m_facePhotoIx, jsonString); // 返回 lastInsertId

    item->ix = newIx; // 记录 IX
    m_items.push_back(std::move(item));
    update();
}

void ImageEditor::addCircle(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3)
{
    auto item = std::make_unique<CircleItem>(QPointF(x1,y1), QPointF(x2,y2),QPointF(x3,y3), m_penColor, m_penWidth);

    QString jsonString = QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact);
    int newIx = AppDb::instance().insertDrawInfo(m_facePhotoIx, jsonString);

    item->ix = newIx;
    m_items.push_back(std::move(item));
    update();
}

QVariantMap ImageEditor::hitTestPoint(qreal x, qreal y)
{
    QVariantMap result;
    result["itemIx"] = -1;
    result["pointIndex"] = -1;
    QPointF pos = mapViewToImage(x, y);
    qreal threshold = 10.0 * qMax(imageToViewScaleX(), imageToViewScaleY());

    // 逆序遍历，优先检测最上层的图形
    for (int i = m_items.size() - 1; i >= 0; --i) {
        // 只有 SmoothCurveItem 支持点位编辑
        auto curve = dynamic_cast<SmoothCurveItem*>(m_items[i].get());
        if (curve) {
            int ptIdx = curve->hitTestPoint(pos, threshold); // 需要在类中实现此函数
            if (ptIdx != -1) {
                result["itemIx"] = curve->ix; // 数据库 IX
                result["internalIdx"] = i;    // 在 m_items 中的索引，方便 updatePoint 使用
                result["pointIndex"] = ptIdx;
                break;
            }
        }
    }
    return result;
}

SmoothCurveItem* ImageEditor::smoothCurveAt(int internalIdx)
{
    if (internalIdx < 0 || internalIdx >= static_cast<int>(m_items.size()))
        return nullptr;
    return dynamic_cast<SmoothCurveItem*>(m_items[internalIdx].get());
}

QString ImageEditor::smoothHitTypeName(SmoothCurveItem::SmoothEditHitType type)
{
    switch (type) {
    case SmoothCurveItem::EditHitPoint: return QStringLiteral("point");
    case SmoothCurveItem::EditHitMove: return QStringLiteral("move");
    case SmoothCurveItem::EditHitScale: return QStringLiteral("scale");
    case SmoothCurveItem::EditHitRotate: return QStringLiteral("rotate");
    default: return QStringLiteral("none");
    }
}

QVariantMap ImageEditor::hitTestSmoothEdit(qreal x, qreal y)
{
    QVariantMap result;
    result["internalIdx"] = -1;
    result["hitType"] = QStringLiteral("none");
    result["pointIndex"] = -1;

    const QPointF pos = mapViewToImage(x, y);
    const qreal threshold = 10.0 * qMax(imageToViewScaleX(), imageToViewScaleY());

    for (int i = static_cast<int>(m_items.size()) - 1; i >= 0; --i) {
        auto *curve = dynamic_cast<SmoothCurveItem*>(m_items[i].get());
        if (!curve || !curve->showControlPoints)
            continue;

        const auto hit = curve->hitTestEdit(pos, threshold);
        if (hit.type == SmoothCurveItem::EditHitNone)
            continue;

        result["internalIdx"] = i;
        result["hitType"] = smoothHitTypeName(hit.type);
        result["pointIndex"] = hit.pointIndex;
        break;
    }
    return result;
}

void ImageEditor::beginSmoothEdit(int internalIdx, const QString &hitType, int pointIndex, qreal x, qreal y)
{
    auto *curve = smoothCurveAt(internalIdx);
    if (!curve)
        return;

    clearAllEditStates();
    curve->isBeingEdited = true;

    m_editItemIdx = internalIdx;
    m_editHitType = hitType;
    m_editPointIdx = pointIndex;
    m_editSnapshot = curve->points;
    m_editPivot = curve->centroid();
    m_editPressPos = mapViewToImage(x, y);

    if (hitType == QStringLiteral("scale")) {
        m_editStartDistance = QLineF(m_editPivot, m_editPressPos).length();
        if (m_editStartDistance < 1.0)
            m_editStartDistance = 1.0;
    } else if (hitType == QStringLiteral("rotate")) {
        const QPointF v = m_editPressPos - m_editPivot;
        m_editStartAngle = std::atan2(v.y(), v.x());
    }
}

void ImageEditor::updateSmoothEdit(qreal x, qreal y)
{
    auto *curve = smoothCurveAt(m_editItemIdx);
    if (!curve)
        return;

    const QPointF pos = mapViewToImage(x, y);
    if (m_editHitType == QStringLiteral("point")) {
        if (m_editPointIdx >= 0 && m_editPointIdx < curve->points.size())
            curve->points[m_editPointIdx] = pos;
    } else if (m_editHitType == QStringLiteral("move")) {
        curve->points = m_editSnapshot;
        curve->translatePoints(pos - m_editPressPos);
    } else if (m_editHitType == QStringLiteral("scale")) {
        const qreal dist = QLineF(m_editPivot, pos).length();
        qreal factor = dist / m_editStartDistance;
        factor = qBound(0.05, factor, 20.0);
        curve->points = m_editSnapshot;
        curve->scalePointsUniform(factor, m_editPivot);
    } else if (m_editHitType == QStringLiteral("rotate")) {
        const QPointF v = pos - m_editPivot;
        const qreal angle = std::atan2(v.y(), v.x());
        curve->points = m_editSnapshot;
        curve->rotatePoints(angle - m_editStartAngle, m_editPivot);
    }
    update();
}

void ImageEditor::endSmoothEdit()
{
    if (m_editItemIdx >= 0)
        syncItemToDb(m_editItemIdx);

    if (auto *curve = smoothCurveAt(m_editItemIdx))
        curve->isBeingEdited = false;

    m_editItemIdx = -1;
    m_editHitType.clear();
    m_editPointIdx = -1;
    m_editSnapshot.clear();
    update();
}

bool ImageEditor::syncGroupContourToAnchor()
{
    if (m_groupContourInternalIdx < 0 || m_image.isNull())
        return false;

    auto *curve = smoothCurveAt(m_groupContourInternalIdx);
    if (!curve)
        return false;

    QString custId;
    int groupId = 0;
    QString dirType;
    if (!AppDb::instance().resolveFacePhotoContext(m_facePhotoIx, &custId, &groupId, &dirType))
        return false;

    const QJsonArray pixelPts = FaceContourMath::pixelsToJson(curve->points);

    QString source = QStringLiteral("manual");
    const auto existing = AppDb::instance().getGroupContourDrawInfo(custId, groupId, dirType);
    if (existing.first >= 0) {
        const QJsonObject oldObj = QJsonDocument::fromJson(existing.second.toUtf8()).object();
        const QString oldSource = oldObj.value(QStringLiteral("source")).toString();
        if (oldSource == QStringLiteral("auto") || oldSource == QStringLiteral("template"))
            source = QStringLiteral("manual");
        else if (!oldSource.isEmpty())
            source = oldSource;
    }

    const QJsonObject json = FaceContourMath::makeGroupContourJson(
        source, pixelPts, curve->color, curve->width);
    const QString compact = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
    if (!AppDb::instance().upsertGroupContourOnAnchor(custId, groupId, dirType, compact))
        return false;

    const auto updated = AppDb::instance().getGroupContourDrawInfo(custId, groupId, dirType);
    curve->ix = updated.first;
    return true;
}

bool ImageEditor::syncItemToDb(int internalIdx)
{
    if (internalIdx == m_groupContourInternalIdx)
        return syncGroupContourToAnchor();

    if (internalIdx < 0 || internalIdx >= static_cast<int>(m_items.size()))
        return false;

    BaseDrawingItem *item = m_items[internalIdx].get();
    if (!item || item->ix < 0)
        return false;

    const QString json = QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact);
    return AppDb::instance().updateDrawInfo(item->ix, json);
}

void ImageEditor::setEditState(int internalIdx, bool editing)
{
    // 1. 基础范围检查
    if (internalIdx >= 0 && internalIdx < (int)m_items.size()) {

        // 2. 尝试将基类指针转换为派生类 SmoothCurveItem 指针
        auto curve = dynamic_cast<SmoothCurveItem*>(m_items[internalIdx].get());

        // 3. 只有转换成功（说明是平滑曲线类型）才执行逻辑
        if (curve) {
            curve->isBeingEdited = editing;
            update(); // 仅在状态确实改变时触发重绘
        }
    }
}

void ImageEditor::clearAllEditStates()
{
    for (auto& item : m_items) {
        item->isBeingEdited = false;
    }
    update();
}

void ImageEditor::updatePoint(int internalIdx, int pointIdx, qreal x, qreal y)
{
    if (internalIdx >= 0 && internalIdx < m_items.size()) {
        auto curve = dynamic_cast<SmoothCurveItem*>(m_items[internalIdx].get());
        if (curve && pointIdx >= 0 && pointIdx < curve->points.size()) {
            curve->points[pointIdx] = mapViewToImage(x, y);
            update(); // 触发界面重绘
        }
    }
}

void ImageEditor::paint(QPainter *painter)
{
    if (m_image.isNull()) return;
    const qreal iw = m_image.width();
    const qreal ih = m_image.height();
    const QRectF dest(0, 0, width(), height());
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(dest, m_image);

    if (!m_shapesVisible)
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    const qreal sx = imageToViewScaleX();
    const qreal sy = imageToViewScaleY();
    if (!qFuzzyCompare(sx, 1.0) || !qFuzzyCompare(sy, 1.0)) {
        painter->save();
        painter->scale(sx, sy);
        for (const auto &item : m_items)
            item->paint(painter);
        painter->restore();
    } else {
        for (const auto &item : m_items)
            item->paint(painter);
    }
}
void ImageEditor::eraseAt(qreal x, qreal y, qreal eraserRadius)
{
    QPointF pos = mapViewToImage(x, y);
    if (eraserRadius <= 0.0)
        eraserRadius = 24.0;
    eraserRadius *= qMax(imageToViewScaleX(), imageToViewScaleY());

    // 逆序遍历，从最上层开始检测
    for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
        if ((*it)->isHit(pos, eraserRadius)) {

            // --- 核心逻辑：检查是否为不可删除的平滑曲线 ---
            // 尝试转换为 SmoothCurveItem
            auto isSmoothCurve = dynamic_cast<SmoothCurveItem*>(it->get());

            if (isSmoothCurve) {
                // 如果是平滑曲线，直接跳过，不执行删除
                continue;
            }

            // --- 如果是普通线段或圆，则执行删除 ---
            // 1. 从数据库中删除
            if ((*it)->ix != -1) {
                AppDb::instance().deleteDrawInfoByIX((*it)->ix);
            }

            // 2. 从内存列表中删除
            // 注意：rbegin().base() 转换稍微有点绕，也可以记录索引后在循环外删
            m_items.erase(std::next(it).base());

            update(); // 触发重绘
            break;    // 橡皮擦一次只擦除一个图形
        }
    }
}

void ImageEditor::loadFromDb(int facePhotoIx, const QString &dirType)
{
    m_items.clear();
    m_groupContourInternalIdx = -1;

    QString custId;
    int groupId = 0;
    QString photoDirType;
    AppDb::instance().resolveFacePhotoContext(facePhotoIx, &custId, &groupId, &photoDirType);
    const QString side = photoDirType.isEmpty() ? dirType : photoDirType;

    auto results = AppDb::instance().getAllDrawInfos(facePhotoIx);
    for (const auto& res : results) {
        QJsonObject obj = QJsonDocument::fromJson(res.second.toUtf8()).object();
        if (FaceContourMath::isSmoothCurve(obj))
            continue;

        auto item = createItemFromJson(obj, false);
        if (item) {
            item->ix = res.first;
            m_items.push_back(std::move(item));
        }
    }

    if (!custId.isEmpty() && groupId > 0)
        loadGroupContourFromAnchor(custId, groupId, side);

    update();
}

bool ImageEditor::loadGroupContourFromAnchor(const QString &custId, int groupId, const QString &dirType)
{
    // Group face contour lives on anchor FacePhoto_IX only. See design_doc/group_contour_storage.md
    const auto pair = AppDb::instance().getGroupContourDrawInfo(custId, groupId, dirType);
    if (pair.first < 0 || m_image.isNull())
        return false;

    QJsonObject obj = QJsonDocument::fromJson(pair.second.toUtf8()).object();
    if (!FaceContourMath::isGroupSmoothCurve(obj))
        return false;

    const QJsonArray ptsArray = obj.value(QStringLiteral("points")).toArray();
    const QColor color(obj.value(QStringLiteral("color")).toString());
    const int weight = obj.value(QStringLiteral("weight")).toInt(3);

    QVector<QPointF> pts;
    const QString source = obj.value(QStringLiteral("source")).toString();
    const QString coordSpace = obj.value(QStringLiteral("coordSpace")).toString();
    if (coordSpace.isEmpty() || coordSpace == QLatin1String(FaceContourMath::kCoordPixel)) {
        pts = FaceContourMath::jsonToPixels(ptsArray);
        // 旧版误把 LibFA logical768 当像素写入 source=auto 时的一次性修正
        if (source == QLatin1String("auto") && !pts.isEmpty() && m_image.width() > 1200) {
            int maxCoord = 0;
            for (const QPointF &p : pts) {
                maxCoord = qMax(maxCoord, static_cast<int>(qMax(p.x(), p.y())));
            }
            if (maxCoord <= 768)
                pts = FaceContourMath::logicalJsonToPixels(ptsArray, m_image.width(), m_image.height());
        }
    } else if (coordSpace == QLatin1String(FaceContourMath::kCoordLogical768)) {
        // 旧数据：LibFA logical768 → 当前图像素
        pts = FaceContourMath::logicalJsonToPixels(ptsArray, m_image.width(), m_image.height());
    } else {
        // 更早期：768×1024 模板坐标或已是像素
        const QString source = obj.value(QStringLiteral("source")).toString();
        if (source.isEmpty()) {
            pts = FaceContourMath::templateTopLeft1024JsonToPixels(
                ptsArray, m_image.width(), m_image.height());
        } else {
            pts = FaceContourMath::jsonToPixels(ptsArray);
        }
    }
    if (pts.isEmpty())
        return false;

    auto item = std::make_unique<SmoothCurveItem>(pts, color, weight);
    item->ix = pair.first;
    m_items.push_back(std::move(item));
    m_groupContourInternalIdx = static_cast<int>(m_items.size()) - 1;
    return true;
}

std::unique_ptr<BaseDrawingItem> ImageEditor::createItemFromJson(const QJsonObject &obj, bool scaleFromStandard)
{
    QString type = obj["type"].toString();
    QJsonArray ptsArray = obj["points"].toArray();

    // 解析基础属性
    // 注意：模板中颜色字段名为 "color"，宽度为 "weight"
    QColor color(obj["color"].toString());
    int weight = obj["weight"].toInt();


    QVector<QPointF> pts;
    for (int i = 0; i < ptsArray.size(); ++i) {
        QJsonObject pObj = ptsArray[i].toObject();
        pts << QPointF(pObj["x"].toDouble(), pObj["y"].toDouble());
    }
    if (pts.isEmpty()) return nullptr;

    if (type == "smooth_curve") {
        if (scaleFromStandard) {
            // 调用【构造函数 B】：执行 768x1024 -> m_image.size() 的映射
            return std::make_unique<SmoothCurveItem>(pts, m_image.size(), color, weight);
        } else {
            // 调用【构造函数 A】：直接加载（坐标已是像素坐标）
            return std::make_unique<SmoothCurveItem>(pts, color, weight);
        }
    }
    else if (type == "line") {
        return std::make_unique<LineItem>(pts[0],pts[1],color, weight);
    } else if (type == "circle") {
        return std::make_unique<CircleItem>(pts[0],pts[1],pts[2],color, weight);
    }

    return nullptr;
}

void ImageEditor::setAllItemsEditMode(bool enabled)
{
    for (auto& item : m_items) {
        auto curve = dynamic_cast<SmoothCurveItem*>(item.get());
        if (curve) {
            curve->showControlPoints = enabled; // 新增属性：仅控制方块显示
        }
    }
    update();
}

