#include "FaceAnalyseManager.h"

#include "AppDb.h"
#include "AppConfig.h"
#include "FaceContourMath.h"
#include "LibFA.h"
#include "MM_Const_Define.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QPainter>
#include <QtConcurrent>
#include <QUrl>
#include <algorithm>
#include <cmath>

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

void FaceAnalyseManager::clearPendingAutoMarkChoice()
{
    pendingChoiceActive_ = false;
    pendingCustomerId_.clear();
    pendingGroupId_ = 0;
    pendingPreviousL_.clear();
    pendingPreviousR_.clear();
    pendingHadPreviousL_ = false;
    pendingHadPreviousR_ = false;
    pendingTemplateL_.clear();
    pendingTemplateR_.clear();
    pendingNewOkL_ = false;
    pendingNewOkR_ = false;
    pendingSummaryL_.clear();
    pendingSummaryR_.clear();
}

bool FaceAnalyseManager::pendingAutoMarkSideSucceeded(bool isLeft) const
{
    return isLeft ? pendingNewOkL_ : pendingNewOkR_;
}

QString FaceAnalyseManager::pendingAutoMarkSideSummary(bool isLeft) const
{
    return isLeft ? pendingSummaryL_ : pendingSummaryR_;
}

QString FaceAnalyseManager::pendingAutoMarkRevertLabel(bool isLeft) const
{
    if (isLeft ? pendingHadPreviousL_ : pendingHadPreviousR_)
        return QStringLiteral("使用原定位结果");
    return QStringLiteral("使用默认模板");
}

bool FaceAnalyseManager::confirmAutoMarkSideChoice(bool isLeft, bool keepNew)
{
    if (!pendingChoiceActive_ || pendingCustomerId_.isEmpty() || pendingGroupId_ <= 0)
        return false;

    if (keepNew)
        return true;

    const QString dirType = isLeft ? QStringLiteral(LEFT) : QStringLiteral(RIGHT);
    const QString &previousJson = isLeft ? pendingPreviousL_ : pendingPreviousR_;
    const bool hadPrevious = isLeft ? pendingHadPreviousL_ : pendingHadPreviousR_;
    const QString &templateJson = isLeft ? pendingTemplateL_ : pendingTemplateR_;

    if (hadPrevious && !previousJson.isEmpty()) {
        return AppDb::instance().upsertGroupContourOnAnchor(
            pendingCustomerId_, pendingGroupId_, dirType, previousJson);
    }

    const QString path = AppDb::instance().anchorPhotoLocalPath(
        pendingCustomerId_, pendingGroupId_, dirType);
    const QSize imgSize = readImagePixelSizeLibFA(path);
    const QString json = templateFallbackJson(templateJson, imgSize);
    if (json.isEmpty())
        return false;
    return AppDb::instance().upsertGroupContourOnAnchor(
        pendingCustomerId_, pendingGroupId_, dirType, json);
}

void FaceAnalyseManager::finalizeAutoMarkChoice()
{
    clearPendingAutoMarkChoice();
}

void FaceAnalyseManager::autoMarkGroup(const QString &customerId, int groupId, bool fromWorkflow)
{
    if (busy_) {
        emit errorMessage(QStringLiteral("正在定位中，请稍候"));
        return;
    }
    if (customerId.isEmpty() || groupId <= 0) {
        emit autoMarkFinished(false, QStringLiteral("无效的客户或组号"), false);
        return;
    }

    clearPendingAutoMarkChoice();

    const GroupContourMeta metaL = AppDb::instance().getGroupContourMeta(customerId, groupId, LEFT);
    const GroupContourMeta metaR = AppDb::instance().getGroupContourMeta(customerId, groupId, RIGHT);
    const QString pathL = AppDb::instance().anchorPhotoLocalPath(customerId, groupId, LEFT);
    const QString pathR = AppDb::instance().anchorPhotoLocalPath(customerId, groupId, RIGHT);

    const QString templateL = AppDb::instance().getTemplateInfo(QStringLiteral(LEFT));
    const QString templateR = AppDb::instance().getTemplateInfo(QStringLiteral(RIGHT));

    QString previousL;
    QString previousR;
    bool hadPreviousL = false;
    bool hadPreviousR = false;
    if (!fromWorkflow) {
        const auto pairL = AppDb::instance().getGroupContourDrawInfo(customerId, groupId, LEFT);
        if (pairL.first >= 0 && !pairL.second.isEmpty()) {
            previousL = pairL.second;
            hadPreviousL = true;
        }
        const auto pairR = AppDb::instance().getGroupContourDrawInfo(customerId, groupId, RIGHT);
        if (pairR.first >= 0 && !pairR.second.isEmpty()) {
            previousR = pairR.second;
            hadPreviousR = true;
        }
    }

    qCInfo(lcFaceAnalyse) << "autoMarkGroup start"
                          << "customer:" << customerId
                          << "group:" << groupId
                          << "fromWorkflow:" << fromWorkflow
                          << "pathL:" << pathL
                          << "pathR:" << pathR
                          << "lockedL:" << sideLockedForAutoMark(metaL)
                          << "lockedR:" << sideLockedForAutoMark(metaR);

    setBusy(true);

    auto *watcher = new QFutureWatcher<GroupMarkResult>(this);
    connect(watcher, &QFutureWatcher<GroupMarkResult>::finished, this, [this, watcher, customerId, groupId,
                                                                        fromWorkflow, previousL, previousR,
                                                                        hadPreviousL, hadPreviousR,
                                                                        templateL, templateR]() {
        const GroupMarkResult result = watcher->result();
        watcher->deleteLater();

        if (!result.detectorOk) {
            setBusy(false);
            emit autoMarkFinished(false, result.detectorError, false);
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
            emit autoMarkFinished(false, detail, false);
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
        if (!okL || !okR) {
            emit autoMarkFinished(false, detail + QStringLiteral("；保存轮廓失败"), false);
            return;
        }

        const bool needsResultChoice = !fromWorkflow;
        if (needsResultChoice) {
            pendingChoiceActive_ = true;
            pendingCustomerId_ = customerId;
            pendingGroupId_ = groupId;
            pendingPreviousL_ = previousL;
            pendingPreviousR_ = previousR;
            pendingHadPreviousL_ = hadPreviousL;
            pendingHadPreviousR_ = hadPreviousR;
            pendingTemplateL_ = templateL;
            pendingTemplateR_ = templateR;
            pendingNewOkL_ = result.left.dllOk && !result.left.skipped;
            pendingNewOkR_ = result.right.dllOk && !result.right.skipped;
            pendingSummaryL_ = msgL;
            pendingSummaryR_ = msgR;
        }

        emit autoMarkFinished(true, detail, needsResultChoice);
    });

    watcher->setFuture(QtConcurrent::run([metaL, metaR, pathL, pathR, templateL, templateR, fromWorkflow]() -> GroupMarkResult {
        GroupMarkResult out;
        if (!initFaceDetector()) {
            out.detectorOk = false;
            out.detectorError = QStringLiteral("人脸检测初始化失败");
            qCWarning(lcFaceAnalyse) << "initFaceDetector failed";
            return out;
        }
        out.detectorOk = true;
        const bool lockTemplate = fromWorkflow;
        out.left = markSideNoDb(QStringLiteral(LEFT), pathL, templateL,
                              lockTemplate && sideLockedForAutoMark(metaL));
        out.right = markSideNoDb(QStringLiteral(RIGHT), pathR, templateR,
                               lockTemplate && sideLockedForAutoMark(metaR));
        return out;
    }));
}

namespace {

using AnalyseByFileFn = T_ANA_RESULT (*)(char *, char *, int *, int, int, int);

struct SkinAnalyseJob {
    QString capType;
    int analyseFunction = 0;
    int reportType = 0;
    int sourceType = SOURCE_RGB;
    int nMin = 75;
    int nMax = 125;
    QString itemLabel;
    AnalyseByFileFn run = nullptr;
};

struct GroupAnalyseResult {
    bool success = false;
    QString message;
    int okCount = 0;
    int failCount = 0;
};

AnalyseByFileFn apiForAnalyseFunction(int analyseFunction)
{
    switch (analyseFunction) {
    case MM_ANALYSE_SPOTS:
        return analyseSpotsByFile;
    case MM_ANALYSE_PORES:
        return analysePoresByFile;
    case MM_ANALYSE_EVENNESS:
        return analyseEvennessByFile;
    case MM_ANALYSE_WRINKLE:
        return analyseWrinkleByFile;
    case MM_ANALYSE_ACNES:
        return analyseAcnesByFile;
    default:
        return nullptr;
    }
}

bool isLibFaAnalyseFunction(int analyseFunction)
{
    return analyseFunction >= MM_ANALYSE_SPOTS && analyseFunction <= MM_ANALYSE_ACNES;
}

int sourceTypeForCapType(const QString &capType)
{
    if (capType == MM_UV)
        return SOURCE_UV365;
    if (capType == MM_PL)
        return SOURCE_PL_POSITIVE;
    if (capType == MM_NPL)
        return SOURCE_PL_NEGATIVE;
    return SOURCE_RGB;
}

QString labelForAnalyseFunction(int analyseFunction)
{
    switch (analyseFunction) {
    case MM_ANALYSE_SPOTS:
        return QStringLiteral("色斑");
    case MM_ANALYSE_PORES:
        return QStringLiteral("毛孔");
    case MM_ANALYSE_EVENNESS:
        return QStringLiteral("均匀度");
    case MM_ANALYSE_WRINKLE:
        return QStringLiteral("皱纹");
    case MM_ANALYSE_ACNES:
        return QStringLiteral("痤疮");
    case MM_ANALYSE_MOISTURE:
        return QStringLiteral("水分");
    default:
        return QStringLiteral("分析");
    }
}

QVector<SkinAnalyseJob> buildSkinAnalyseJobsFromDb(QString *errorOut)
{
    QVector<SkinAnalyseJob> jobs;
    const QVector<FacePhotoAnalyseMapEntry> mapRows = AppDb::instance().getFacePhotoAnalyseMap();
    if (mapRows.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("T_FacePhoto_Map 未配置分析映射");
        return jobs;
    }

    for (const FacePhotoAnalyseMapEntry &row : mapRows) {
        if (!isLibFaAnalyseFunction(row.analyseFunction))
            continue;

        SkinAnalyseJob job;
        job.capType = row.capType;
        job.analyseFunction = row.analyseFunction;
        job.reportType = row.reportType;
        job.sourceType = sourceTypeForCapType(row.capType);
        job.itemLabel = labelForAnalyseFunction(row.analyseFunction);
        job.run = apiForAnalyseFunction(row.analyseFunction);
        if (!job.run)
            continue;
        jobs.append(job);
    }

    if (jobs.isEmpty() && errorOut)
        *errorOut = QStringLiteral("T_FacePhoto_Map 中没有可执行的 LibFA64 分析项");
    return jobs;
}

int customerAgeYears(const Customer &customer)
{
    const QDate bd = QDate::fromString(customer.Cust_Birthday, QStringLiteral("yyyy-MM-dd"));
    if (!bd.isValid())
        return 30;
    return qMax(1, bd.daysTo(QDate::currentDate()) / 365);
}

bool loadSidePxl(const QString &custId, int groupId, const QString &dirType, QVector<int> *outPxl)
{
    if (!outPxl)
        return false;
    const auto pair = AppDb::instance().getGroupContourDrawInfo(custId, groupId, dirType);
    if (pair.first < 0 || pair.second.isEmpty())
        return false;

    const QJsonObject obj = QJsonDocument::fromJson(pair.second.toUtf8()).object();
    if (!FaceContourMath::isGroupSmoothCurve(obj))
        return false;

    const QVector<QPointF> pixels = FaceContourMath::jsonToPixels(
        obj.value(QStringLiteral("points")).toArray());
    if (pixels.size() < 3)
        return false;

    outPxl->clear();
    outPxl->reserve(pixels.size() * 2);
    for (const QPointF &p : pixels) {
        outPxl->append(qRound(p.x()));
        outPxl->append(qRound(p.y()));
    }
    return true;
}

QString overlayOutputPath(const QString &groupDir, const QString &photoName)
{
    const QString analyseDir = groupDir + QStringLiteral("/analyse");
    QDir().mkpath(analyseDir);
    const QString base = QFileInfo(photoName).completeBaseName();
    return QDir(groupDir).filePath(QStringLiteral("analyse/") + base + QStringLiteral("_overlay.jpg"));
}

QString sideLabel(const QString &dirType)
{
    return dirType == LEFT ? QStringLiteral("左脸") : QStringLiteral("右脸");
}

bool runOneSkinAnalyse(const SkinAnalyseJob &job,
                       const QString &dirType,
                       const QVector<int> &pxl,
                       int age,
                       int gender,
                       const QString &custId,
                       int groupId,
                       QString *warnOut)
{
    FacePhoto photo;
    if (!AppDb::instance().findPhotoInGroupByCapType(
            custId, groupId, dirType, job.capType, &photo)) {
        if (warnOut)
            *warnOut = QStringLiteral("%1 %2 未找到照片").arg(sideLabel(dirType), job.capType);
        return false;
    }

    const QString inPath = AppDb::instance().photoFilePath(photo);
    if (inPath.isEmpty() || !QFile::exists(inPath)) {
        if (warnOut)
            *warnOut = QStringLiteral("%1 图片不存在：%2").arg(sideLabel(dirType), inPath);
        return false;
    }
    if (pxl.size() < 6) {
        if (warnOut)
            *warnOut = QStringLiteral("%1 ROI 无效").arg(sideLabel(dirType));
        return false;
    }

    setExtraInfo(age, gender, job.sourceType);

    int nMin = job.nMin;
    int nMax = job.nMax;
    AppConfig::instance().analyseMinMaxForJob(
        job.analyseFunction, job.capType, age, gender, &nMin, &nMax);
    int conVal = 80;
    int minArea = 30;
    int maxArea = 100;
    AppConfig::instance().analyseRunOptionsForJob(
        job.analyseFunction, job.capType, age, &conVal, &minArea, &maxArea);
    setAnalyseRunOptions(conVal, minArea, maxArea);

    const QString outPath = overlayOutputPath(
        AppDb::instance().groupFolderPath(custId, groupId), photo.Photo_Name);

    QByteArray inBytes = inPath.toLocal8Bit();
    QByteArray outBytes = outPath.toLocal8Bit();
    QVector<int> pxlCopy = pxl;
    const int pxlCount = pxlCopy.size();
    T_ANA_RESULT r = job.run(
        inBytes.data(), outBytes.data(), pxlCopy.data(), pxlCount, nMin, nMax);

    if (!AppDb::instance().upsertAnalyseInfo(photo.IX, job.analyseFunction, r.value, r.percent)) {
        if (warnOut)
            *warnOut = QStringLiteral("保存分析结果失败：%1").arg(AppDb::instance().lastErrorText());
        return false;
    }

    qCInfo(lcFaceAnalyse) << "skinAnalyse ok"
                          << sideLabel(dirType)
                          << job.capType
                          << "function:" << job.analyseFunction
                          << "value:" << r.value
                          << "percent:" << r.percent
                          << "overlay:" << outPath;
    return true;
}

GroupAnalyseResult runGroupSkinAnalyseWorker(FaceAnalyseManager *mgr,
                                       const QString &customerId,
                                       int groupId)
{
    GroupAnalyseResult out;
    QString mapError;
    const QVector<SkinAnalyseJob> jobs = buildSkinAnalyseJobsFromDb(&mapError);
    if (jobs.isEmpty()) {
        out.message = mapError.isEmpty() ? QStringLiteral("未找到分析配置") : mapError;
        return out;
    }
    const int totalJobs = jobs.size() * 2;
    int done = 0;

    Customer customer;
    if (!AppDb::instance().findCustomerByCustId(customerId, &customer)) {
        out.message = QStringLiteral("未找到客户信息");
        return out;
    }

    QVector<int> pxlL;
    QVector<int> pxlR;
    if (!loadSidePxl(customerId, groupId, LEFT, &pxlL) || !loadSidePxl(customerId, groupId, RIGHT, &pxlR)) {
        out.message = QStringLiteral("读取左右脸 ROI 失败，请先完成区域定位");
        return out;
    }

    if (!AppDb::instance().deleteGroupAnalyseInfo(customerId, groupId)) {
        out.message = QStringLiteral("清除旧分析结果失败");
        return out;
    }

    const int age = customerAgeYears(customer);
    const int gender = customer.Cust_Gender > 0 ? customer.Cust_Gender : 1;
    QStringList warnings;
    const QString sides[] = { QStringLiteral(LEFT), QStringLiteral(RIGHT) };

    for (const QString &side : sides) {
        const QVector<int> &pxl = side == LEFT ? pxlL : pxlR;
        for (const SkinAnalyseJob &job : jobs) {
            ++done;
            const QString progressLabel = sideLabel(side) + QStringLiteral(" ") + job.capType
                    + QStringLiteral("（") + job.itemLabel + QStringLiteral("）");
            QMetaObject::invokeMethod(mgr, "notifyGroupAnalyseProgress", Qt::QueuedConnection,
                                      Q_ARG(int, done), Q_ARG(int, totalJobs), Q_ARG(QString, progressLabel));

            QString warn;
            if (runOneSkinAnalyse(job, side, pxl, age, gender, customerId, groupId, &warn)) {
                ++out.okCount;
            } else {
                ++out.failCount;
                if (!warn.isEmpty())
                    warnings.append(warn);
                qCWarning(lcFaceAnalyse) << "skinAnalyse fail" << progressLabel << warn;
            }
        }
    }

    out.success = out.okCount > 0;
    if (out.failCount == 0) {
        out.message = QStringLiteral("皮肤分析完成，共 %1 项。").arg(out.okCount);
    } else if (out.okCount > 0) {
        out.message = QStringLiteral("皮肤分析部分完成：成功 %1 项，失败 %2 项。\n%3")
                          .arg(out.okCount)
                          .arg(out.failCount)
                          .arg(warnings.join(QStringLiteral("；")));
    } else {
        out.message = QStringLiteral("皮肤分析失败。\n") + warnings.join(QStringLiteral("；"));
    }
    return out;
}

} // namespace

void FaceAnalyseManager::analyseGroup(const QString &customerId, int groupId)
{
    if (busy_) {
        emit errorMessage(QStringLiteral("正在处理中，请稍候"));
        return;
    }
    if (customerId.isEmpty() || groupId <= 0) {
        emit groupAnalyseFinished(false, QStringLiteral("无效的客户或组号"));
        return;
    }
    if (groupNeedsAutoMark(customerId, groupId)) {
        emit groupAnalyseFinished(false, QStringLiteral("请先完成左右脸区域定位"));
        return;
    }
    if (!ensureDetector()) {
        emit groupAnalyseFinished(false, QStringLiteral("人脸检测初始化失败"));
        return;
    }

    setBusy(true);
    qCInfo(lcFaceAnalyse) << "analyseGroup start customer:" << customerId << "group:" << groupId;

    auto *watcher = new QFutureWatcher<GroupAnalyseResult>(this);
    connect(watcher, &QFutureWatcher<GroupAnalyseResult>::finished, this, [this, watcher]() {
        const GroupAnalyseResult result = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        qCInfo(lcFaceAnalyse) << "analyseGroup done success:" << result.success
                              << "ok:" << result.okCount
                              << "fail:" << result.failCount;
        emit groupAnalyseFinished(result.success, result.message);
    });

    watcher->setFuture(QtConcurrent::run([this, customerId, groupId]() {
        return runGroupSkinAnalyseWorker(this, customerId, groupId);
    }));
}

void FaceAnalyseManager::notifyGroupAnalyseProgress(int done, int total, const QString &label)
{
    emit groupAnalyseProgress(done, total, label);
}

bool FaceAnalyseManager::photoHasAnalyseOverlay(int facePhotoIx) const
{
    return !AppDb::instance().analyseOverlayPathForPhoto(facePhotoIx).isEmpty();
}

QUrl FaceAnalyseManager::photoAnalyseOverlayUrl(int facePhotoIx) const
{
    const QString path = AppDb::instance().analyseOverlayPathForPhoto(facePhotoIx);
    if (path.isEmpty())
        return QUrl();
    return QUrl::fromLocalFile(path);
}

namespace {

QString localFilePathFromMaybeUrl(const QString &pathOrUrl)
{
    if (pathOrUrl.isEmpty())
        return QString();
    if (pathOrUrl.startsWith(QLatin1String("file:"), Qt::CaseInsensitive))
        return QUrl(pathOrUrl).toLocalFile();
    return pathOrUrl;
}

/// FaceRecon 侧脸 UV 贴图布局（实测 2664×3552 侧图 → 7104×3552 贴图）：
/// [左黑边 448][L 2664][中缝 880][R 2664][右黑边 448]
void faceReconPasteOffsets(int photoW, int atlasW, int *outLeftX, int *outRightX)
{
    const double refW = 2664.0;
    int xl = static_cast<int>(std::lround(448.0 * photoW / refW));
    int xr = static_cast<int>(std::lround(3992.0 * photoW / refW));
    if (xl < 0)
        xl = 0;
    if (xr < 0)
        xr = 0;
    if (xl + photoW > atlasW)
        xl = std::max(0, atlasW - photoW);
    if (xr + photoW > atlasW)
        xr = std::max(0, atlasW - photoW);
    if (outLeftX)
        *outLeftX = xl;
    if (outRightX)
        *outRightX = xr;
}

qint64 newestMTime(const QStringList &paths)
{
    qint64 newest = 0;
    for (const QString &p : paths) {
        if (p.isEmpty())
            continue;
        const QFileInfo fi(p);
        if (!fi.exists())
            continue;
        newest = std::max(newest, fi.lastModified().toMSecsSinceEpoch());
    }
    return newest;
}

bool pasteSide(QImage *atlas, const QImage &side, int x)
{
    if (!atlas || side.isNull() || x < 0)
        return false;
    QImage src = side;
    if (src.format() != QImage::Format_RGB888 && src.format() != QImage::Format_ARGB32
        && src.format() != QImage::Format_RGB32) {
        src = src.convertToFormat(QImage::Format_RGB888);
    }
    if (src.height() != atlas->height() || src.width() > atlas->width() - x) {
        src = src.scaled(std::min(src.width(), atlas->width() - x), atlas->height(),
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    QPainter painter(atlas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(x, 0, src);
    painter.end();
    return true;
}

} // namespace

QUrl FaceAnalyseManager::ensurePairAnalyseAtlasUrl(const QString &groupDir,
                                                   const QString &textureStem,
                                                   int facePhotoIxL,
                                                   int facePhotoIxR,
                                                   const QString &baseAtlasLocalPath) const
{
    if (groupDir.isEmpty() || textureStem.isEmpty())
        return QUrl();

    const QString overlayL = AppDb::instance().analyseOverlayPathForPhoto(facePhotoIxL);
    const QString overlayR = AppDb::instance().analyseOverlayPathForPhoto(facePhotoIxR);
    if (overlayL.isEmpty() && overlayR.isEmpty())
        return QUrl();

    const QString baseAtlas = localFilePathFromMaybeUrl(baseAtlasLocalPath);
    if (baseAtlas.isEmpty() || !QFileInfo::exists(baseAtlas))
        return QUrl();

    const QString analyseDir = QDir(groupDir).filePath(QStringLiteral("analyse"));
    QDir().mkpath(analyseDir);
    const QString outPath = QDir(analyseDir).filePath(textureStem + QStringLiteral("_atlas.jpg"));

    QStringList sources;
    sources << baseAtlas << overlayL << overlayR;
    const qint64 srcNewest = newestMTime(sources);
    const QFileInfo outInfo(outPath);
    if (outInfo.exists() && outInfo.lastModified().toMSecsSinceEpoch() >= srcNewest)
        return QUrl::fromLocalFile(outPath);

    QImage atlas = QImage(baseAtlas);
    if (atlas.isNull())
        return QUrl();
    if (atlas.format() != QImage::Format_RGB32 && atlas.format() != QImage::Format_ARGB32
        && atlas.format() != QImage::Format_RGB888) {
        atlas = atlas.convertToFormat(QImage::Format_RGB32);
    } else {
        atlas = atlas.copy();
    }

    FacePhoto photoL;
    FacePhoto photoR;
    const bool haveL = facePhotoIxL > 0
                       && AppDb::instance().findFacePhotoByIx(facePhotoIxL, &photoL);
    const bool haveR = facePhotoIxR > 0
                       && AppDb::instance().findFacePhotoByIx(facePhotoIxR, &photoR);

    QImage imgL;
    QImage imgR;
    if (!overlayL.isEmpty())
        imgL = QImage(overlayL);
    else if (haveL) {
        const QString p = AppDb::instance().photoFilePath(photoL);
        if (!p.isEmpty())
            imgL = QImage(p);
    }
    if (!overlayR.isEmpty())
        imgR = QImage(overlayR);
    else if (haveR) {
        const QString p = AppDb::instance().photoFilePath(photoR);
        if (!p.isEmpty())
            imgR = QImage(p);
    }

    const int photoW = !imgL.isNull() ? imgL.width()
                                      : (!imgR.isNull() ? imgR.width() : 0);
    if (photoW <= 0)
        return QUrl();

    int xl = 0;
    int xr = 0;
    faceReconPasteOffsets(photoW, atlas.width(), &xl, &xr);

    bool painted = false;
    if (!overlayL.isEmpty() && !imgL.isNull())
        painted = pasteSide(&atlas, imgL, xl) || painted;
    if (!overlayR.isEmpty() && !imgR.isNull())
        painted = pasteSide(&atlas, imgR, xr) || painted;
    if (!painted)
        return QUrl();

    if (!atlas.save(outPath, "JPG", 92)) {
        qCWarning(lcFaceAnalyse) << "save analyse atlas failed:" << outPath;
        return QUrl();
    }
    qCInfo(lcFaceAnalyse) << "analyse atlas:" << outPath << "xl" << xl << "xr" << xr;
    return QUrl::fromLocalFile(outPath);
}
