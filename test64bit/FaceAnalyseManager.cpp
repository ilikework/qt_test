#include "FaceAnalyseManager.h"

#include "AppDb.h"
#include "FaceContourMath.h"
#include "LibFA.h"
#include "MM_Const_Define.h"

#include <QColor>
#include <QFile>
#include <QFutureWatcher>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QtConcurrent>

Q_LOGGING_CATEGORY(lcFaceAnalyse, "FaceAnalyse")

namespace {

struct SideMarkPayload {
    QString dirType;
    QString jsonCompact;
    bool skipped = false;
    bool dllOk = false;
    bool fileMissing = false;
    int dllPointCount = -1;
    QSize imageSize;
    QString imagePath;
};

struct GroupMarkResult {
    bool detectorOk = false;
    QString detectorError;
    SideMarkPayload left;
    SideMarkPayload right;
};

static QJsonArray contourToPixelJson(const T_CONTOUR &c, int imageWidth, int imageHeight)
{
    if (!c.x || !c.y || c.count <= 0)
        return QJsonArray();
    return FaceContourMath::libfaContourToPixelJson(c.count, c.x, c.y, imageWidth, imageHeight);
}

static QSize readImagePixelSizeLibFA(const QString &imagePath)
{
    int w = 0;
    int h = 0;
    const QByteArray pathBytes = imagePath.toLocal8Bit();
    if (libfaImageFileSize(pathBytes.constData(), &w, &h) && w > 0 && h > 0)
        return QSize(w, h);
    return QSize();
}

static QString templateFallbackJson(const QString &templateJson, const QSize &imgSize)
{
    if (!imgSize.isValid() || templateJson.isEmpty())
        return QString();

    QJsonObject obj = QJsonDocument::fromJson(templateJson.toUtf8()).object();
    const QVector<QPointF> pixels = FaceContourMath::templateTopLeft1024JsonToPixels(
        obj.value(QStringLiteral("points")).toArray(), imgSize.width(), imgSize.height());
    const QColor color(obj.value(QStringLiteral("color")).toString(QStringLiteral("#ff0000")));
    const int weight = obj.value(QStringLiteral("weight")).toInt(3);
    const QJsonObject out = FaceContourMath::makeGroupContourJson(
        QStringLiteral("template"), FaceContourMath::pixelsToJson(pixels), color, weight);
    return QString::fromUtf8(QJsonDocument(out).toJson(QJsonDocument::Compact));
}

static SideMarkPayload markSideNoDb(const QString &dirType,
                                  const QString &imagePath,
                                  const QString &templateJson,
                                  bool lockedTemplate)
{
    SideMarkPayload out;
    out.dirType = dirType;
    out.imagePath = imagePath;
    if (lockedTemplate) {
        out.skipped = true;
        out.dllOk = true;
        qCInfo(lcFaceAnalyse) << dirType << "skip: template locked";
        return out;
    }

    if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
        out.fileMissing = true;
        qCWarning(lcFaceAnalyse) << dirType << "anchor image missing:" << imagePath;
        return out;
    }

    const QSize imgSize = readImagePixelSizeLibFA(imagePath);
    out.imageSize = imgSize;
    if (!imgSize.isValid()) {
        out.fileMissing = true;
        qCWarning(lcFaceAnalyse) << dirType << "cannot read image:" << imagePath;
        return out;
    }

    QByteArray pathBytes = imagePath.toLocal8Bit();
    T_CONTOUR c{0, nullptr, nullptr};
    if (dirType == LEFT)
        c = autoMarkLeftFaceByFile(pathBytes.data());
    else
        c = autoMarkRightFaceByFile(pathBytes.data());

    out.dllPointCount = c.count;
    qCInfo(lcFaceAnalyse) << dirType
                          << "path:" << imagePath
                          << "cvSize:" << imgSize.width() << "x" << imgSize.height()
                          << "LibFA logical points:" << c.count;

    if (c.count > 0) {
        const QJsonObject json = FaceContourMath::makeGroupContourJson(
            QStringLiteral("auto"),
            contourToPixelJson(c, imgSize.width(), imgSize.height()),
            Qt::red, 3);
        out.jsonCompact = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
        out.dllOk = true;
        qCInfo(lcFaceAnalyse) << dirType << "result: auto OK";
    } else {
        out.jsonCompact = templateFallbackJson(templateJson, imgSize);
        out.dllOk = false;
        if (out.jsonCompact.isEmpty())
            qCWarning(lcFaceAnalyse) << dirType << "result: LibFA failed and template fallback empty";
        else
            qCWarning(lcFaceAnalyse) << dirType << "result: LibFA failed -> template fallback";
    }
    freeContour(&c);
    return out;
}

} // namespace

namespace {

bool sideContourFinalized(const GroupContourMeta &meta)
{
    if (!meta.hasContour)
        return false;
    // source=template：自动定位已失败并用默认轮廓，不再重试
    if (meta.source == QLatin1String("template"))
        return true;
    // 旧版 logical768 等：需重做并写成像素
    if (!meta.coordSpace.isEmpty() && meta.coordSpace != QLatin1String(FaceContourMath::kCoordPixel))
        return false;
    return meta.source == QLatin1String("auto") || meta.source == QLatin1String("manual");
}

bool sideLockedForAutoMark(const GroupContourMeta &meta)
{
    return meta.hasContour && meta.source == QLatin1String("template");
}

} // namespace

FaceAnalyseManager::FaceAnalyseManager(QObject *parent)
    : QObject{parent}
{
}

void FaceAnalyseManager::setBusy(bool on)
{
    if (busy_ == on)
        return;
    busy_ = on;
    emit busyChanged();
}

bool FaceAnalyseManager::ensureDetector()
{
    if (detectorReady_)
        return true;
    if (!initFaceDetector()) {
        emit errorMessage(QStringLiteral("initFaceDetector 失败，请确认 shape_predictor_68_face_landmarks.dat 在程序目录"));
        return false;
    }
    detectorReady_ = true;
    return true;
}

bool FaceAnalyseManager::deleteCustomerGroup(const QString &customerId, int groupId)
{
    return AppDb::instance().deleteGroup(customerId, groupId);
}

QString FaceAnalyseManager::contourState(const QString &customerId, int groupId, const QString &dirType) const
{
    const GroupContourMeta meta = AppDb::instance().getGroupContourMeta(customerId, groupId, dirType);
    if (!meta.hasContour)
        return QStringLiteral("none");
    if (meta.source == QStringLiteral("manual"))
        return QStringLiteral("manual");
    if (meta.source == QStringLiteral("auto"))
        return QStringLiteral("auto");
    if (meta.source == QStringLiteral("template"))
        return QStringLiteral("template");
    return QStringLiteral("none");
}

bool FaceAnalyseManager::groupNeedsAutoMark(const QString &customerId, int groupId) const
{
    const QString sides[] = { QStringLiteral(LEFT), QStringLiteral(RIGHT) };
    for (const QString &side : sides) {
        const GroupContourMeta meta = AppDb::instance().getGroupContourMeta(customerId, groupId, side);
        if (!sideContourFinalized(meta))
            return true;
    }
    return false;
}

void FaceAnalyseManager::autoMarkGroup(const QString &customerId, int groupId)
{
    if (busy_) {
        emit errorMessage(QStringLiteral("正在定位中，请稍候"));
        return;
    }
    if (customerId.isEmpty() || groupId <= 0) {
        emit autoMarkFinished(false, QStringLiteral("无效的客户或组号"));
        return;
    }

    const GroupContourMeta metaL = AppDb::instance().getGroupContourMeta(customerId, groupId, LEFT);
    const GroupContourMeta metaR = AppDb::instance().getGroupContourMeta(customerId, groupId, RIGHT);
    const QString pathL = AppDb::instance().anchorPhotoLocalPath(customerId, groupId, LEFT);
    const QString pathR = AppDb::instance().anchorPhotoLocalPath(customerId, groupId, RIGHT);

    const QString templateL = AppDb::instance().getTemplateInfo(QStringLiteral(LEFT));
    const QString templateR = AppDb::instance().getTemplateInfo(QStringLiteral(RIGHT));

    qCInfo(lcFaceAnalyse) << "autoMarkGroup start"
                          << "customer:" << customerId
                          << "group:" << groupId
                          << "pathL:" << pathL
                          << "pathR:" << pathR
                          << "lockedL:" << sideLockedForAutoMark(metaL)
                          << "lockedR:" << sideLockedForAutoMark(metaR);

    setBusy(true);

    auto *watcher = new QFutureWatcher<GroupMarkResult>(this);
    connect(watcher, &QFutureWatcher<GroupMarkResult>::finished, this, [this, watcher, customerId, groupId]() {
        const GroupMarkResult result = watcher->result();
        watcher->deleteLater();

        if (!result.detectorOk) {
            setBusy(false);
            emit autoMarkFinished(false, result.detectorError);
            return;
        }

        auto sideReady = [](const SideMarkPayload &side) -> bool {
            if (side.skipped)
                return true;
            if (side.fileMissing || side.jsonCompact.isEmpty())
                return false;
            return true;
        };

        auto sideSummary = [](const QString &label, const SideMarkPayload &side) -> QString {
            if (side.skipped)
                return QStringLiteral("%1：已锁定默认轮廓（跳过）").arg(label);
            if (side.fileMissing)
                return QStringLiteral("%1：锚点图片未找到").arg(label);
            if (side.jsonCompact.isEmpty())
                return QStringLiteral("%1：定位失败").arg(label);
            if (side.dllOk)
                return QStringLiteral("%1：自动定位成功").arg(label);
            if (side.dllPointCount == 0)
                return QStringLiteral("%1：自动定位失败（LibFA 未检测到人脸，0 个点），已用默认轮廓").arg(label);
            return QStringLiteral("%1：自动定位失败，已用默认轮廓").arg(label);
        };

        const QString msgL = sideSummary(QStringLiteral("左脸"), result.left);
        const QString msgR = sideSummary(QStringLiteral("右脸"), result.right);
        const QString detail = msgL + QStringLiteral("；") + msgR;

        setBusy(false);

        if (!sideReady(result.left) || !sideReady(result.right)) {
            emit autoMarkFinished(false, detail);
            return;
        }

        auto writeSide = [&](const SideMarkPayload &side) -> bool {
            if (side.skipped)
                return true;
            return AppDb::instance().upsertGroupContourOnAnchor(
                customerId, groupId, side.dirType, side.jsonCompact);
        };

        const bool okL = writeSide(result.left);
        const bool okR = writeSide(result.right);
        qCInfo(lcFaceAnalyse) << "autoMarkGroup done"
                              << "writeL:" << okL
                              << "writeR:" << okR
                              << "detail:" << detail;
        if (okL && okR)
            emit autoMarkFinished(true, detail);
        else
            emit autoMarkFinished(false, detail + QStringLiteral("；保存轮廓失败"));
    });

    watcher->setFuture(QtConcurrent::run([metaL, metaR, pathL, pathR, templateL, templateR]() -> GroupMarkResult {
        GroupMarkResult out;
        if (!initFaceDetector()) {
            out.detectorOk = false;
            out.detectorError = QStringLiteral("人脸检测初始化失败");
            qCWarning(lcFaceAnalyse) << "initFaceDetector failed";
            return out;
        }
        out.detectorOk = true;
        out.left = markSideNoDb(QStringLiteral(LEFT), pathL, templateL, sideLockedForAutoMark(metaL));
        out.right = markSideNoDb(QStringLiteral(RIGHT), pathR, templateR, sideLockedForAutoMark(metaR));
        return out;
    }));
}
