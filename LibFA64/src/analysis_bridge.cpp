#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "libfa_compat.h"
#include "../vendor/lib.h"

#include "libfa_internal.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace libfa {
namespace internal {
namespace {

struct AnalysisRegion {
    cv::Rect rc;
    std::vector<int> pnX;
    std::vector<int> pnY;
    byte **gray = nullptr;
};

static cv::Rect polygonBounds(const int *pxl, int pxl_cnt, const cv::Size &imageSize)
{
    if (!pxl || pxl_cnt < 4)
        return cv::Rect(0, 0, imageSize.width, imageSize.height);

    int minX = pxl[0];
    int maxX = pxl[0];
    int minY = pxl[1];
    int maxY = pxl[1];
    for (int i = 2; i < pxl_cnt; i += 2) {
        minX = std::min(minX, pxl[i]);
        maxX = std::max(maxX, pxl[i]);
        minY = std::min(minY, pxl[i + 1]);
        maxY = std::max(maxY, pxl[i + 1]);
    }

    minX = std::max(0, minX);
    minY = std::max(0, minY);
    maxX = std::min(imageSize.width - 1, maxX);
    maxY = std::min(imageSize.height - 1, maxY);

    return cv::Rect(minX, minY, std::max(1, maxX - minX + 1), std::max(1, maxY - minY + 1));
}

static bool buildRegion(const cv::Mat &bgr, const int *pxl, int pxl_cnt, AnalysisRegion &region)
{
    region.rc = polygonBounds(pxl, pxl_cnt, bgr.size());
    const int width = region.rc.width;
    const int height = region.rc.height;
    if (width <= 0 || height <= 0)
        return false;

    get_mem2D(&region.gray, height, width);

    for (int x = region.rc.x, ii = 0; x < region.rc.x + width; ++x, ++ii) {
        for (int y = region.rc.y + height - 1, j = 0; y >= region.rc.y; --y, ++j) {
            const cv::Vec3b px = bgr.at<cv::Vec3b>(y, x);
            region.gray[j][ii] = byte(rgb2y(px[2], px[1], px[0]));
        }
    }

    const int pointCount = pxl_cnt / 2;
    region.pnX.resize(pointCount);
    region.pnY.resize(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        region.pnX[i] = pxl[2 * i] - region.rc.x;
        region.pnY[i] = (region.rc.y + height - 1) - pxl[2 * i + 1];
    }
    return true;
}

static void releaseRegion(AnalysisRegion &region)
{
    free_mem2Dbyte(region.gray);
    region.gray = nullptr;
}

/* MagicFace analysis.ini [RGBEvennessConst] R1..R24 (OpenCV BGR) */
static const unsigned char kEvennessBgr[24][3] = {
    {220, 255, 220}, /*  0 绿 */
    {180, 220, 180},
    {140, 180, 140},
    {100, 140, 100},
    { 60, 100,  60},
    { 20, 250, 250}, /*  5 黄 */
    { 20, 220, 220},
    { 20, 180, 180},
    { 20, 140, 140},
    { 20, 100, 100},
    {100, 100, 255}, /* 10 橙 */
    { 60,  60, 225},
    { 20,  20, 180},
    {  1, 140, 140},
    {  1, 100, 100},
    {  1,  60,  60},
    {  1,   1,   1}, /* 16+ 深 */
    {  1,   1,   1},
    {  1,   1,   1},
    {  1,   1,   1},
    {  1,   1,   1},
    {  1,   1,   1},
    {  1,   1,   1},
    {  1,   1,   1},
};

static void overlayGrayResult(cv::Mat &bgr, const AnalysisRegion &region, bool evennessMode)
{
    const int width = region.rc.width;
    const int height = region.rc.height;
    for (int x = 0, ii = 0; x < width; ++x, ++ii) {
        for (int y = height - 1, j = 0; y >= 0; --y, ++j) {
            const byte v = region.gray[j][ii];
            bool draw = false;
            if (evennessMode)
                draw = (v >= 0 && v < 24);
            else
                draw = (v > 0);

            if (!draw)
                continue;

            const int imgX = region.rc.x + x;
            const int imgY = region.rc.y + y;
            cv::Vec3b &px = bgr.at<cv::Vec3b>(imgY, imgX);
            if (evennessMode) {
                const int level = std::min(23, std::max(0, static_cast<int>(v)));
                px[0] = kEvennessBgr[level][0];
                px[1] = kEvennessBgr[level][1];
                px[2] = kEvennessBgr[level][2];
            } else {
                px[0] = 255;
                px[1] = 0;
                px[2] = 0;
            }
        }
    }
}

static T_ANA_RESULT makeEmptyResult()
{
    T_ANA_RESULT r = {0, 0};
    return r;
}

} // namespace

T_ANA_RESULT analysePoresByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    T_ANA_RESULT result = makeEmptyResult();
    if (!inFile || !outFile || !pxl || pxl_cnt < 4)
        return result;

    cv::Mat bgr = cv::imread(inFile);
    if (bgr.empty())
        return result;

    AnalysisRegion region;
    if (!buildRegion(bgr, pxl, pxl_cnt, region))
        return result;

    int portCount = 0;
    int area = 0;
    const int conVal = 80;
    maokong_ana_new(region.gray, region.rc.width, region.rc.height,
                    region.pnX.data(), region.pnY.data(), region.pnX.size(),
                    &portCount, &area, nMin, nMax, conVal, nullptr, 1);

    overlayGrayResult(bgr, region, false);
    cv::imwrite(outFile, bgr);

    result.value = portCount;
    result.percent = (area > 0) ? (portCount * 10000 / area) : 0;
    releaseRegion(region);
    return result;
}

T_ANA_RESULT analyseSpotsByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    T_ANA_RESULT result = makeEmptyResult();
    if (!inFile || !outFile || !pxl || pxl_cnt < 4)
        return result;

    cv::Mat bgr = cv::imread(inFile);
    if (bgr.empty())
        return result;

    AnalysisRegion region;
    if (!buildRegion(bgr, pxl, pxl_cnt, region))
        return result;

    int portCount = 0;
    int area = 0;
    const int maxArea = 100;
    const int minArea = 30;
    const int conVal = 80;
    sports_ana_new(region.gray, region.rc.width, region.rc.height,
                   region.pnX.data(), region.pnY.data(), region.pnX.size(),
                   &portCount, &area, nMin, nMax, maxArea, minArea, conVal, nullptr, 1);

    overlayGrayResult(bgr, region, false);
    cv::imwrite(outFile, bgr);

    result.value = portCount;
    result.percent = (area > 0) ? (portCount * 10000 / area) : 0;
    releaseRegion(region);
    return result;
}

T_ANA_RESULT analyseWrinkleByFile(const char *inFile, const char *outFile,
                                  const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    T_ANA_RESULT result = makeEmptyResult();
    if (!inFile || !outFile || !pxl || pxl_cnt < 4)
        return result;

    cv::Mat bgr = cv::imread(inFile);
    if (bgr.empty())
        return result;

    AnalysisRegion region;
    if (!buildRegion(bgr, pxl, pxl_cnt, region))
        return result;

    int area = 0;
    const int maxArea = 100;
    const int minArea = 30;
    const int conVal = 80;
    const int wrinkleScore = winkel_ana_new1(region.gray, region.rc.width, region.rc.height,
                                             region.pnX.data(), region.pnY.data(), region.pnX.size(),
                                             &area, nMin, nMax, maxArea, minArea, conVal, nullptr);

    overlayGrayResult(bgr, region, false);
    cv::imwrite(outFile, bgr);

    result.value = wrinkleScore;
    result.percent = wrinkleScore * 100 / 128;
    releaseRegion(region);
    return result;
}

T_ANA_RESULT analyseAcnesByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    (void)nMax;
    T_ANA_RESULT result = makeEmptyResult();
    if (!inFile || !outFile || !pxl || pxl_cnt < 4)
        return result;

    cv::Mat bgr = cv::imread(inFile);
    if (bgr.empty())
        return result;

    AnalysisRegion region;
    if (!buildRegion(bgr, pxl, pxl_cnt, region))
        return result;

    int portCount = 0;
    int area = 0;
    porphyrin_ana_new(region.gray, region.rc.width, region.rc.height,
                      region.pnX.data(), region.pnY.data(), region.pnX.size(),
                      &portCount, &area, nMin, nullptr);

    overlayGrayResult(bgr, region, false);
    cv::imwrite(outFile, bgr);

    result.value = portCount;
    result.percent = (area > 0) ? (portCount * 10000 / area) : 0;
    releaseRegion(region);
    return result;
}

T_ANA_RESULT analyseEvennessByFile(const char *inFile, const char *outFile,
                                   const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    (void)nMin;
    (void)nMax;
    T_ANA_RESULT result = makeEmptyResult();
    if (!inFile || !outFile || !pxl || pxl_cnt < 4)
        return result;

    cv::Mat bgr = cv::imread(inFile);
    if (bgr.empty())
        return result;

    AnalysisRegion region;
    if (!buildRegion(bgr, pxl, pxl_cnt, region))
        return result;

    std::vector<float> columnResult(region.rc.width, 0.0f);
    const int evennessScore = evenness_ana(region.gray, region.pnX.data(), region.pnY.data(),
                                           region.pnX.size(), region.rc.width, region.rc.height,
                                           columnResult.data());

    overlayGrayResult(bgr, region, true);
    cv::imwrite(outFile, bgr);

    result.value = evennessScore;
    result.percent = evennessScore * 100;
    releaseRegion(region);
    return result;
}

} // namespace internal
} // namespace libfa
