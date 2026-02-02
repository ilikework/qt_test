#include "ImageEditor.h"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
#include "AppDb.h"

void ImageEditor::init(const int IX, const QString &dirType)
{
    m_facePhotoIx = IX;
    loadFromDb(IX, dirType);
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
    if (m_image.load(QUrl(path).toLocalFile())) {

        // 设置图片的原始像素大小
        setImplicitWidth(m_image.width());
        setImplicitHeight(m_image.height());
        update();
        emit sourceChanged(); // 触发 QML 的逻辑
    }
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
    QPointF pos(x, y);
    qreal threshold = 10.0; // 点击方块的容错范围

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
            curve->points[pointIdx] = QPointF(x, y);
            update(); // 触发界面重绘
        }
    }
}

void ImageEditor::paint(QPainter *painter)
{
    if (m_image.isNull()) return;
    QRectF rect(0, 0, width(), height());
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(rect, m_image);

    // 2. 根据开关决定是否绘制线条和圆
    if (m_shapesVisible) {
        painter->setRenderHint(QPainter::Antialiasing);
        for (const auto& item : m_items) {
            item->paint(painter);
        }
    }
}
void ImageEditor::eraseAt(qreal x, qreal y)
{
    QPointF pos(x, y);
    qreal threshold = 5.0; // 橡皮擦的灵敏度

    // 逆序遍历，从最上层开始检测
    for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
        if ((*it)->isHit(pos, threshold)) {

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
    bool hasSmoothCurve = false;

    // 1. 先尝试从正式绘图表获取该照片的所有图形
    auto results = AppDb::instance().getAllDrawInfos(facePhotoIx);
    for (const auto& res : results) {
        QJsonObject obj = QJsonDocument::fromJson(res.second.toUtf8()).object();
        if (obj["type"].toString() == "smooth_curve") hasSmoothCurve = true;

        // 这里 createItemFromJson 内部会处理从 768x1024 到实际像素的转换（如果需要）
        // 假设正式表存的已经是图片像素坐标，这里缩放比例应为 1:1
        auto item = createItemFromJson(obj, false); // 增加一个参数控制是否需要初始缩放
        if (item) {
            item->ix = res.first;
            m_items.push_back(std::move(item));
        }
    }

    // 2. 如果没有找到平滑曲线，则从模板加载并“像素化”后存入正式表
    if (!hasSmoothCurve) {
        QString templateJson = AppDb::instance().getTemplateInfo(dirType);
        if (!templateJson.isEmpty()) {
            QJsonObject TmplObj = QJsonDocument::fromJson(templateJson.toUtf8()).object();

            // A. 先按模板逻辑（768x1024 -> 实际像素）创建 Item
            auto templateItem = createItemFromJson(TmplObj, true);

            if (templateItem) {
                // B. 将这个已经缩放到实际像素的 Item 转化为 JSON 字符串
                // 此时 toJson() 生成的点坐标就是 3000x4000 这种实际像素了
                QString finalJson = QJsonDocument(templateItem->toJson()).toJson(QJsonDocument::Compact);

                // C. 将这个针对当前图片的“定制化”JSON 存入正式表，获取 IX
                int newIx = AppDb::instance().insertDrawInfo(facePhotoIx, finalJson);

                if (newIx != -1) {
                    templateItem->ix = newIx;
                    m_items.push_back(std::move(templateItem));
                }
            }
        }
    }
    update();
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

