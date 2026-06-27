#include "libfa_internal.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/opencv/cv_image.h>

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

namespace libfa {
namespace internal {
namespace {

static int g_modelLoaded = 0;
static dlib::shape_predictor g_shapePredictor;

static cv::Mat grayStretch(const cv::Mat &img)
{
    cv::Mat stretched(img.rows, img.cols, CV_8UC3);
    const int nHeight = 200;
    const int nLimit = 70;
    const float fStretch = 255.0f / static_cast<float>(nHeight - nLimit);

    for (int i = 0; i < img.cols; ++i) {
        for (int j = 0; j < img.rows; ++j) {
            const cv::Vec3b px = img.at<cv::Vec3b>(j, i);
            for (int c = 0; c < 3; ++c) {
                const int v = px[c];
                if (v <= nLimit) {
                    stretched.at<cv::Vec3b>(j, i)[c] = 0;
                } else if (v >= nHeight) {
                    stretched.at<cv::Vec3b>(j, i)[c] = 255;
                } else {
                    stretched.at<cv::Vec3b>(j, i)[c] =
                        static_cast<uchar>(fStretch * static_cast<float>(v - nLimit) + 0.5f);
                }
            }
        }
    }
    return stretched;
}

/* 来自 DreamMirror.cpp FaceMask（第四版） */
static void faceMask(float *marks, float *faceMasks)
{
    const float d1 = sqrtf((marks[2] - marks[0]) * (marks[2] - marks[0]) +
                           (marks[3] - marks[1]) * (marks[3] - marks[1]));
    const float d2 = sqrtf((marks[4] - marks[2]) * (marks[4] - marks[2]) +
                           (marks[5] - marks[3]) * (marks[5] - marks[3]));
    const float d3 = sqrtf((marks[6] - marks[4]) * (marks[6] - marks[4]) +
                           (marks[7] - marks[5]) * (marks[7] - marks[5]));
    const float d4 = sqrtf((marks[8] - marks[6]) * (marks[8] - marks[6]) +
                           (marks[9] - marks[7]) * (marks[9] - marks[7]));
    const float d5 = sqrtf((marks[10] - marks[8]) * (marks[10] - marks[8]) +
                           (marks[11] - marks[9]) * (marks[11] - marks[9]));
    const float d = (d1 + d2 + d3 + d4 + d5) / 20.0f;

    faceMasks[0] = marks[42];
    faceMasks[1] = marks[41] - d * 1.5f;
    faceMasks[2] = marks[38];
    faceMasks[3] = marks[39] - d * 2.5f;
    faceMasks[4] = marks[36];
    faceMasks[5] = marks[37] - d * 2.5f;
    faceMasks[8] = (marks[42] + marks[44]) / 2.0f;
    faceMasks[9] = marks[55] - (marks[48] - marks[38]) * 0.85f;
    faceMasks[6] = (marks[36] + marks[38]) / 2.0f;
    faceMasks[7] = marks[39] - (marks[39] - faceMasks[9]) * 0.65f;
    faceMasks[10] = (marks[50] + marks[48]) / 2.0f;
    faceMasks[11] = marks[49] - (marks[49] - faceMasks[9]) * 0.65f;

    if (faceMasks[9] < 0) {
        faceMasks[9] = 0;
        faceMasks[7] = marks[49] * 0.25f;
        faceMasks[11] = marks[39] * 0.25f;
    }

    faceMasks[12] = marks[50];
    faceMasks[13] = marks[51] - d * 2.5f;
    faceMasks[14] = marks[48];
    faceMasks[15] = marks[49] - d * 2.5f;
    faceMasks[16] = marks[44];
    faceMasks[17] = marks[47] - d * 1.5f;
    faceMasks[18] = marks[54] + (marks[84] - marks[54]) * 0.6f;
    faceMasks[19] = (marks[45] + marks[85]) / 2.0f;
    faceMasks[20] = marks[84];
    faceMasks[21] = marks[85] + 0.5f * d;
    faceMasks[22] = marks[94];
    faceMasks[23] = marks[95] + d;
    faceMasks[24] = marks[90];
    faceMasks[25] = marks[93] + d;
    faceMasks[26] = marks[32] - (marks[32] - marks[90]) * 0.6f;
    faceMasks[27] = (marks[33] + marks[31]) / 2.0f;
    if (faceMasks[27] < faceMasks[25])
        faceMasks[27] = faceMasks[25];
    faceMasks[28] = marks[30] - (marks[30] - marks[28]) / 3.0f - 2.0f * d;
    faceMasks[29] = marks[31] + (marks[29] - marks[31]) / 3.0f;
    faceMasks[30] = marks[28] - (marks[28] - marks[26]) * 2.0f / 3.0f - 2.0f * d;
    faceMasks[31] = marks[29] + (marks[27] - marks[29]) * 2.0f / 3.0f;
    faceMasks[32] = marks[24] - 2.0f * d;
    faceMasks[33] = marks[25];
    faceMasks[34] = marks[108] + (marks[24] - marks[108]) / 3.0f;
    faceMasks[35] = marks[25];
    faceMasks[36] = marks[108] + d * 4.0f / 3.0f;
    faceMasks[37] = marks[67] + (marks[103] - marks[67]) / 3.0f;
    faceMasks[38] = marks[70] + (marks[70] - marks[60]) / 2.0f;
    faceMasks[39] = marks[61];
    faceMasks[40] = marks[70];
    faceMasks[41] = marks[59];
    faceMasks[42] = (marks[68] + marks[70]) / 2.0f;
    faceMasks[43] = marks[61] - 50.0f;
    faceMasks[44] = marks[60];
    faceMasks[45] = marks[61] + (marks[67] - marks[61]) / 3.0f;
    faceMasks[46] = (marks[64] + marks[62]) / 2.0f;
    faceMasks[47] = marks[61] - 50.0f;
    faceMasks[48] = marks[62];
    faceMasks[49] = marks[59];
    faceMasks[50] = marks[62] - (marks[60] - marks[62]) / 2.0f;
    faceMasks[51] = marks[61];
    faceMasks[52] = marks[96] - d * 4.0f / 3.0f;
    faceMasks[53] = marks[67] + (marks[103] - marks[67]) / 3.0f;
    faceMasks[54] = marks[96] - (marks[96] - marks[8]) / 3.0f;
    faceMasks[55] = marks[9];
    faceMasks[56] = marks[8] + 2.0f * d;
    faceMasks[57] = marks[9];
    faceMasks[58] = marks[4] + (marks[6] - marks[4]) * 2.0f / 3.0f + 2.0f * d;
    faceMasks[59] = marks[5] + (marks[7] - marks[5]) * 2.0f / 3.0f;
    faceMasks[60] = marks[2] + (marks[4] - marks[2]) / 3.0f + 2.0f * d;
    faceMasks[61] = marks[3] + (marks[5] - marks[3]) / 3.0f;
    faceMasks[62] = marks[0] + (marks[72] - marks[0]) * 0.6f;
    faceMasks[63] = (marks[1] + marks[3]) / 2.0f;
    faceMasks[64] = marks[72];
    faceMasks[65] = marks[83] + d;
    if (faceMasks[63] < faceMasks[65])
        faceMasks[63] = faceMasks[65];
    faceMasks[66] = marks[80];
    faceMasks[67] = marks[81] + d;
    faceMasks[68] = marks[78];
    faceMasks[69] = marks[79] + 0.5f * d;
    faceMasks[70] = marks[54] - (marks[54] - marks[78]) * 0.6f;
    faceMasks[71] = (marks[43] + marks[79]) / 2.0f;
}

/* DreamMirror.cpp LeftFaceMask */
static void leftFaceMask(float *marks, float *faceMasks)
{
    const float d1 = sqrtf((marks[2] - marks[0]) * (marks[2] - marks[0]) +
                           (marks[3] - marks[1]) * (marks[3] - marks[1]));
    const float d2 = sqrtf((marks[4] - marks[2]) * (marks[4] - marks[2]) +
                           (marks[5] - marks[3]) * (marks[5] - marks[3]));
    const float d3 = sqrtf((marks[6] - marks[4]) * (marks[6] - marks[4]) +
                           (marks[7] - marks[5]) * (marks[7] - marks[5]));
    const float d4 = sqrtf((marks[8] - marks[6]) * (marks[8] - marks[6]) +
                           (marks[9] - marks[7]) * (marks[9] - marks[7]));
    const float d5 = sqrtf((marks[10] - marks[8]) * (marks[10] - marks[8]) +
                           (marks[11] - marks[9]) * (marks[11] - marks[9]));
    const float d = (d1 + d2 + d3 + d4 + d5) / 20.0f;

    faceMasks[0] = marks[32] - (marks[32] - marks[90]) / 3.0f;
    faceMasks[1] = marks[33] - (marks[33] - marks[53]) / 2.0f;
    faceMasks[2] = marks[28] - 4.0f * d;
    faceMasks[3] = marks[29];
    faceMasks[4] = marks[24] - 4.0f * d;
    faceMasks[5] = marks[25];
    faceMasks[6] = marks[20];
    faceMasks[7] = marks[25];
    faceMasks[8] = (marks[18] + marks[20]) / 2.0f;
    faceMasks[9] = marks[109] - (marks[103] - marks[67]) / 2.0f;
    faceMasks[10] = marks[70] + (marks[70] - marks[60]) / 2.0f;
    faceMasks[11] = marks[71];
    faceMasks[12] = marks[68];
    faceMasks[13] = marks[61];
    faceMasks[14] = marks[60];
    faceMasks[15] = marks[61];
    faceMasks[16] = marks[54];
    faceMasks[17] = marks[55];
    faceMasks[18] = marks[54] + (marks[84] - marks[54]) / 2.0f;
    faceMasks[19] = marks[55];
    faceMasks[20] = marks[84];
    faceMasks[21] = marks[85] + d;
    faceMasks[22] = (marks[90] + marks[52]) / 2.0f;
    faceMasks[23] = marks[93] + d;
    faceMasks[24] = marks[52] + d;
    faceMasks[25] = marks[53];
}

/* DreamMirror.cpp RightFaceMask */
static void rightFaceMask(float *marks, float *faceMasks)
{
    const float d1 = sqrtf((marks[2] - marks[0]) * (marks[2] - marks[0]) +
                           (marks[3] - marks[1]) * (marks[3] - marks[1]));
    const float d2 = sqrtf((marks[4] - marks[2]) * (marks[4] - marks[2]) +
                           (marks[5] - marks[3]) * (marks[5] - marks[3]));
    const float d3 = sqrtf((marks[6] - marks[4]) * (marks[6] - marks[4]) +
                           (marks[7] - marks[5]) * (marks[7] - marks[5]));
    const float d4 = sqrtf((marks[8] - marks[6]) * (marks[8] - marks[6]) +
                           (marks[9] - marks[7]) * (marks[9] - marks[7]));
    const float d5 = sqrtf((marks[10] - marks[8]) * (marks[10] - marks[8]) +
                           (marks[11] - marks[9]) * (marks[11] - marks[9]));
    const float d = (d1 + d2 + d3 + d4 + d5) / 20.0f;

    faceMasks[0] = marks[0] + (marks[72] - marks[0]) / 3.0f;
    faceMasks[1] = marks[1] - (marks[1] - marks[35]) / 2.0f;
    faceMasks[2] = marks[4] + 4.0f * d;
    faceMasks[3] = marks[5];
    faceMasks[4] = marks[8] + 4.0f * d;
    faceMasks[5] = marks[9];
    faceMasks[6] = marks[12];
    faceMasks[7] = marks[9];
    faceMasks[8] = marks[96];
    faceMasks[9] = marks[67] + (marks[103] - marks[67]) / 3.0f;
    faceMasks[10] = marks[62] - (marks[60] - marks[62]) / 2.0f;
    faceMasks[11] = marks[61];
    faceMasks[12] = marks[64];
    faceMasks[13] = marks[61];
    faceMasks[14] = marks[60];
    faceMasks[15] = marks[61];
    faceMasks[16] = marks[54];
    faceMasks[17] = marks[55];
    faceMasks[18] = marks[54] - (marks[54] - marks[78]) / 2.0f;
    faceMasks[19] = marks[55];
    faceMasks[20] = marks[78];
    faceMasks[21] = marks[79] + d;
    faceMasks[22] = (marks[72] + marks[34]) / 2.0f;
    faceMasks[23] = marks[73] + d;
    faceMasks[24] = marks[34] - d;
    faceMasks[25] = marks[35];
}

enum class FacePose { Front, SideR, SideL };

static int expectedPointCount(FacePose pose)
{
    return (pose == FacePose::Front) ? 36 : 13;
}

static int loadShapeModel(dlib::shape_predictor &sp)
{
    const char *filename = "shape_predictor_68_face_landmarks.dat";
    FILE *fp = nullptr;
    if (fopen_s(&fp, filename, "rb") != 0)
        return -1;
    fclose(fp);
    dlib::deserialize(filename) >> sp;
    return 1;
}

static float clampWorkScale(const cv::Mat &image, float scaleByHeight)
{
    float s = std::min(1.0f, scaleByHeight);
    const int maxSide = std::max(image.cols, image.rows);
    if (maxSide * s > 1600.0f)
        s = 1600.0f / static_cast<float>(maxSide);
    return std::max(0.12f, s);
}

static void bgrToDlibWork(const cv::Mat &bgr, bool mirror, dlib::array2d<dlib::bgr_pixel> &work)
{
    work.set_size(bgr.rows, bgr.cols);
    for (int r = 0; r < bgr.rows; ++r) {
        for (int c = 0; c < bgr.cols; ++c) {
            const int srcC = mirror ? (bgr.cols - c - 1) : c;
            const cv::Vec3b px = bgr.at<cv::Vec3b>(r, srcC);
            work[r][c].blue = px[0];
            work[r][c].green = px[1];
            work[r][c].red = px[2];
        }
    }
}

static dlib::rectangle pickLargestFace(const std::vector<dlib::rectangle> &dets)
{
    if (dets.empty())
        return dlib::rectangle();
    dlib::rectangle best = dets[0];
    long bestArea = best.area();
    for (size_t i = 1; i < dets.size(); ++i) {
        const long area = dets[i].area();
        if (area > bestArea) {
            bestArea = area;
            best = dets[i];
        }
    }
    return best;
}

static cv::CascadeClassifier &loadHaarCascade(const std::string &fileName)
{
    static std::vector<cv::CascadeClassifier> s_cascades;
    static std::vector<std::string> s_names;
    for (size_t i = 0; i < s_names.size(); ++i) {
        if (s_names[i] == fileName)
            return s_cascades[i];
    }
    s_names.push_back(fileName);
    s_cascades.emplace_back();
    if (!s_cascades.back().load(fileName))
        std::fprintf(stderr, "LibFA64: failed to load haar %s\n", fileName.c_str());
    return s_cascades.back();
}

static std::string resolveHaarPath(const char *leafName)
{
    const std::string leaf(leafName);
    const char *prefixes[] = {
        "haarcascades/",
        "D:/opencv/build/etc/haarcascades/",
        "D:/opencv/sources/data/haarcascades/",
    };
    for (const char *prefix : prefixes) {
        const std::string path = std::string(prefix) + leaf;
        FILE *fp = nullptr;
        if (fopen_s(&fp, path.c_str(), "rb") == 0) {
            fclose(fp);
            return path;
        }
    }
    return leaf;
}

static cv::Rect pickLargestCvRect(const std::vector<cv::Rect> &faces)
{
    if (faces.empty())
        return cv::Rect();
    cv::Rect best = faces[0];
    int bestArea = best.area();
    for (size_t i = 1; i < faces.size(); ++i) {
        const int area = faces[i].area();
        if (area > bestArea) {
            bestArea = area;
            best = faces[i];
        }
    }
    return best;
}

static bool detectFaceHaar(const cv::Mat &bgr, dlib::rectangle &outFace)
{
    if (bgr.empty())
        return false;

    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    const int minSide = std::max(60, std::min(bgr.cols, bgr.rows) / 8);
    const cv::Size minSize(minSide, minSide);

    const char *cascadeLeaves[] = {
        "haarcascade_frontalface_alt2.xml",
        "haarcascade_frontalface_alt.xml",
        "haarcascade_profileface.xml",
    };

    cv::Rect best;
    int bestArea = 0;

    auto scan = [&](const cv::Mat &srcGray, bool fromFlipped) {
        for (const char *leaf : cascadeLeaves) {
            cv::CascadeClassifier &cascade = loadHaarCascade(resolveHaarPath(leaf));
            if (cascade.empty())
                continue;
            std::vector<cv::Rect> faces;
            cascade.detectMultiScale(srcGray, faces, 1.08, 3, 0, minSize);
            cv::Rect pick = pickLargestCvRect(faces);
            if (fromFlipped && pick.area() > 0)
                pick.x = gray.cols - pick.x - pick.width;
            if (pick.area() > bestArea) {
                bestArea = pick.area();
                best = pick;
            }
        }
    };

    scan(gray, false);
    cv::Mat flippedGray;
    cv::flip(gray, flippedGray, 1);
    scan(flippedGray, true);

    if (bestArea <= 0)
        return false;

    outFace = dlib::rectangle(best.x, best.y, best.x + best.width, best.y + best.height);
    return true;
}

static dlib::rectangle expandFaceRect(const dlib::rectangle &face, long cols, long rows, float margin = 0.18f)
{
    if (face.area() <= 0)
        return face;
    const long mx = static_cast<long>(face.width() * margin);
    const long my = static_cast<long>(face.height() * margin);
    const long l = std::max(0L, face.left() - mx);
    const long t = std::max(0L, face.top() - my);
    const long r = std::min(cols, face.right() + mx);
    const long b = std::min(rows, face.bottom() + my);
    return dlib::rectangle(l, t, r, b);
}

static bool faceBoxLargeEnough(const dlib::rectangle &face, float fScale, const cv::Mat &image, FacePose pose)
{
    if (face.area() <= 0 || fScale <= 0.0f)
        return false;
    const float origW = face.width() / fScale;
    const float origH = face.height() / fScale;
    const float minWFrac = (pose == FacePose::Front) ? 0.08f : 0.10f;
    const float minHFrac = (pose == FacePose::Front) ? 0.08f : 0.08f;
    return origW >= image.cols * minWFrac && origH >= image.rows * minHFrac;
}

static long scoreDetectAttempt(const dlib::rectangle &face, float fScale, bool fromDlib, bool useStretch,
                               FacePose pose, bool mirror)
{
    const float origW = face.width() / fScale;
    const float origH = face.height() / fScale;
    long score = static_cast<long>(origW * origH);
    if (fromDlib)
        score += score / 2;
    if (useStretch)
        score += 100000L;
    if (pose == FacePose::SideL && mirror)
        score += 50000000L;
    if (pose == FacePose::SideR && !mirror)
        score += 50000000L;
    return score;
}

static bool landmarksLookPlausible(const dlib::full_object_detection &shape, long cols, long rows, FacePose pose)
{
    if (shape.num_parts() < 68)
        return false;
    long minX = shape.part(0).x();
    long maxX = shape.part(0).x();
    long minY = shape.part(0).y();
    long maxY = shape.part(0).y();
    for (unsigned long i = 1; i < shape.num_parts(); ++i) {
        minX = std::min(minX, shape.part(i).x());
        maxX = std::max(maxX, shape.part(i).x());
        minY = std::min(minY, shape.part(i).y());
        maxY = std::max(maxY, shape.part(i).y());
    }
    const long faceW = maxX - minX;
    const long faceH = maxY - minY;
    const long minSpan = (pose == FacePose::Front) ? (cols / 10) : (cols / 5);
    return faceW > minSpan && faceH > rows / 10 && faceW < cols && faceH < rows;
}

struct DetectAttempt {
    float fScale = 1.0f;
    bool mirror = false;
    bool useStretch = true;
    cv::Mat processed;
    dlib::array2d<dlib::bgr_pixel> work;
    dlib::rectangle face;
};

static bool tryDetectFace(const cv::Mat &image, FacePose pose,
                          dlib::frontal_face_detector &detector, DetectAttempt &out)
{
    const float heightTargets[] = {500.0f, 900.0f, 1300.0f, 99999.0f};
    const bool stretchOptions[] = {true, false};
    const bool mirrorOrders[3][2] = {
        {false, true}, /* front */
        {false, true}, /* *_R.jpg：先原图方向 */
        {true, false}, /* *_L.jpg：先镜像（与 R 图对称） */
    };
    const int mirrorOrderIdx = (pose == FacePose::Front) ? 0
                               : (pose == FacePose::SideR) ? 1 : 2;

    long bestScore = 0;
    bool frontDone = false;

    for (float targetH : heightTargets) {
        const float fScale = clampWorkScale(image, targetH / static_cast<float>(image.rows));
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(), fScale, fScale, cv::INTER_CUBIC);

        for (bool useStretch : stretchOptions) {
            const cv::Mat processed = useStretch ? grayStretch(resized) : resized;
            for (int mi = 0; mi < 2; ++mi) {
                const bool mirror = mirrorOrders[mirrorOrderIdx][mi];

                dlib::array2d<dlib::bgr_pixel> work;
                bgrToDlibWork(processed, mirror, work);

                std::vector<dlib::rectangle> dets = detector(work);
                dlib::rectangle face = pickLargestFace(dets);
                const bool fromDlib = (face.area() > 0);

                cv::Mat haarSrc = processed;
                if (mirror)
                    cv::flip(processed, haarSrc, 1);
                if (face.area() <= 0)
                    detectFaceHaar(haarSrc, face);

                if (face.area() <= 0)
                    continue;

                if (!fromDlib)
                    face = expandFaceRect(face, work.nc(), work.nr(),
                                          (pose == FacePose::Front) ? 0.18f : 0.25f);

                if (!faceBoxLargeEnough(face, fScale, image, pose))
                    continue;

                const dlib::full_object_detection shape = g_shapePredictor(work, face);
                if (!landmarksLookPlausible(shape, work.nc(), work.nr(), pose))
                    continue;

                if (pose == FacePose::Front && fromDlib) {
                    out.fScale = fScale;
                    out.mirror = mirror;
                    out.useStretch = useStretch;
                    out.processed = processed.clone();
                    out.face = face;
                    bgrToDlibWork(out.processed, out.mirror, out.work);
                    return true;
                }

                long score = scoreDetectAttempt(face, fScale, fromDlib, useStretch, pose, mirror);
                if (score <= bestScore)
                    continue;

                bestScore = score;
                out.fScale = fScale;
                out.mirror = mirror;
                out.useStretch = useStretch;
                out.processed = processed.clone();
                out.face = face;
                bgrToDlibWork(out.processed, out.mirror, out.work);
                frontDone = true;
            }
        }
    }

    return frontDone || bestScore > 0;
}

static int detectFacePoints(const cv::Mat &image, float *outPoints, FacePose pose)
{
    if (g_modelLoaded <= 0 || image.empty())
        return 0;

    const int pointCount = expectedPointCount(pose);
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    const float fResize = 768.0f / static_cast<float>(image.cols);

    static const int kIndex[68] = {
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 26,
        25, 24, 23, 22, 21, 20, 19, 18, 17, 27, 28, 29, 30, 35, 34, 33, 32, 31, 45, 44, 43, 42,
        47, 46, 39, 38, 37, 36, 41, 40, 54, 53, 52, 51, 50, 49, 48, 59, 58, 57, 56, 55, 64, 63, 62, 61, 60, 67, 66, 65
    };

    DetectAttempt attempt;
    if (!tryDetectFace(image, pose, detector, attempt))
        return 0;

    const dlib::full_object_detection shape = g_shapePredictor(attempt.work, attempt.face);
    float facemask[200] = {};

    if (attempt.mirror) {
        for (unsigned long pt = 0; pt < shape.num_parts(); ++pt) {
            const float x = static_cast<float>(attempt.processed.cols - shape.part(pt).x()) / attempt.fScale;
            const float y = static_cast<float>(shape.part(pt).y()) / attempt.fScale;
            if (pose == FacePose::SideL) {
                facemask[2 * pt] = x;
                facemask[2 * pt + 1] = y;
            } else {
                facemask[2 * kIndex[pt]] = x;
                facemask[2 * kIndex[pt] + 1] = y;
            }
        }
    } else {
        for (unsigned long pt = 0; pt < shape.num_parts(); ++pt) {
            facemask[2 * pt] = static_cast<float>(shape.part(pt).x()) / attempt.fScale;
            facemask[2 * pt + 1] = static_cast<float>(shape.part(pt).y()) / attempt.fScale;
        }
    }

    switch (pose) {
    case FacePose::Front:
        faceMask(facemask, outPoints);
        break;
    case FacePose::SideR:
    case FacePose::SideL:
        leftFaceMask(facemask, outPoints);
        break;
    }

    for (int pt = 0; pt < pointCount; ++pt) {
        outPoints[2 * pt] = fResize * outPoints[2 * pt];
        outPoints[2 * pt + 1] = static_cast<float>(image.rows) - outPoints[2 * pt + 1] - 1.0f;
        outPoints[2 * pt + 1] = fResize * outPoints[2 * pt + 1];
    }
    return pointCount;
}

static T_CONTOUR contourFromFile(const char *fileName, FacePose pose)
{
    T_CONTOUR contour = {0, nullptr, nullptr};
    if (!fileName || g_modelLoaded <= 0)
        return contour;

    const cv::Mat image = cv::imread(fileName);
    if (image.empty())
        return contour;

    float points[200] = {};
    const int count = detectFacePoints(image, points, pose);
    if (count <= 0)
        return contour;

    contour.count = count;
    contour.x = new int[count];
    contour.y = new int[count];
    for (int i = 0; i < count; ++i) {
        contour.x[i] = static_cast<int>(points[2 * i] + 0.5f);
        contour.y[i] = static_cast<int>(points[2 * i + 1] + 0.5f);
    }
    return contour;
}

} // namespace

bool initFaceDetector()
{
    g_modelLoaded = loadShapeModel(g_shapePredictor);
    return g_modelLoaded > 0;
}

T_CONTOUR autoMarkFaceByFile(const char *fileName)
{
    return contourFromFile(fileName, FacePose::Front);
}

T_CONTOUR autoMarkLeftFaceByFile(const char *fileName)
{
    /* 拍摄文件 *_L.jpg：LeftFaceMask + 镜像检测（无 kIndex） */
    return contourFromFile(fileName, FacePose::SideL);
}

T_CONTOUR autoMarkRightFaceByFile(const char *fileName)
{
    /* 拍摄文件 *_R.jpg：LeftFaceMask，检测优先原图方向 */
    return contourFromFile(fileName, FacePose::SideR);
}

void freeContour(T_CONTOUR *contour)
{
    if (!contour)
        return;
    delete[] contour->x;
    delete[] contour->y;
    contour->x = nullptr;
    contour->y = nullptr;
    contour->count = 0;
}

} // namespace internal
} // namespace libfa
