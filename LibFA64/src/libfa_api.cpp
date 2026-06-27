#include "LibFA.h"
#include "libfa_internal.h"
#include <opencv2/imgcodecs.hpp>

LIBFA_API int libfaTestReadImage(const char *path)
{
    if (!path)
        return -1;
    const cv::Mat bgr = cv::imread(path);
    if (bgr.empty())
        return 0;
    return bgr.cols * bgr.rows;
}

LIBFA_API int libfaImageFileSize(const char *path, int *outW, int *outH)
{
    if (!path || !outW || !outH)
        return 0;
    const cv::Mat bgr = cv::imread(path);
    if (bgr.empty())
        return 0;
    *outW = bgr.cols;
    *outH = bgr.rows;
    return 1;
}

LIBFA_API bool initFaceDetector()
{
    return libfa::internal::initFaceDetector();
}

LIBFA_API void setExtraInfo(int age, int gender, int source_type)
{
    libfa::internal::setExtraInfo(age, gender, source_type);
}

LIBFA_API T_ANA_RESULT analysePoresByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax)
{
    return libfa::internal::analysePoresByFile(inFile, outFile, pxl, pxl_cnt, nMin, nMax);
}

LIBFA_API T_ANA_RESULT analyseWrinkleByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax)
{
    return libfa::internal::analyseWrinkleByFile(inFile, outFile, pxl, pxl_cnt, nMin, nMax);
}

LIBFA_API T_ANA_RESULT analyseSpotsByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax)
{
    return libfa::internal::analyseSpotsByFile(inFile, outFile, pxl, pxl_cnt, nMin, nMax);
}

LIBFA_API T_ANA_RESULT analyseAcnesByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax)
{
    return libfa::internal::analyseAcnesByFile(inFile, outFile, pxl, pxl_cnt, nMin, nMax);
}

LIBFA_API T_ANA_RESULT analyseEvennessByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax)
{
    return libfa::internal::analyseEvennessByFile(inFile, outFile, pxl, pxl_cnt, nMin, nMax);
}

LIBFA_API T_CONTOUR autoMarkFaceByFile(char *fileName)
{
    return libfa::internal::autoMarkFaceByFile(fileName);
}

LIBFA_API T_CONTOUR autoMarkLeftFaceByFile(char *fileName)
{
    return libfa::internal::autoMarkLeftFaceByFile(fileName);
}

LIBFA_API T_CONTOUR autoMarkRightFaceByFile(char *fileName)
{
    return libfa::internal::autoMarkRightFaceByFile(fileName);
}

LIBFA_API void freeContour(T_CONTOUR *contour)
{
    libfa::internal::freeContour(contour);
}

LIBFA_API bool generateBrownSunburnPictureByFile(char *inFile, char *outFile)
{
    (void)inFile;
    (void)outFile;
    return false;
}

LIBFA_API bool generateRedBloodPictureByFile(char *inFile, char *outFile)
{
    (void)inFile;
    (void)outFile;
    return false;
}

LIBFA_API bool generateMixedSpotPictureByFile(char *inFile, char *outFile)
{
    (void)inFile;
    (void)outFile;
    return false;
}
