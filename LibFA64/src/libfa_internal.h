#pragma once

#include "LibFA.h"
#include <string>

namespace libfa {
namespace internal {

bool initFaceDetector();
void setExtraInfo(int age, int gender, int source_type);
T_CONTOUR autoMarkFaceByFile(const char *fileName);
T_CONTOUR autoMarkLeftFaceByFile(const char *fileName);
T_CONTOUR autoMarkRightFaceByFile(const char *fileName);
void freeContour(T_CONTOUR *contour);

T_ANA_RESULT analysePoresByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax);
T_ANA_RESULT analyseWrinkleByFile(const char *inFile, const char *outFile,
                                  const int *pxl, int pxl_cnt, int nMin, int nMax);
T_ANA_RESULT analyseSpotsByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax);
T_ANA_RESULT analyseAcnesByFile(const char *inFile, const char *outFile,
                                const int *pxl, int pxl_cnt, int nMin, int nMax);
T_ANA_RESULT analyseEvennessByFile(const char *inFile, const char *outFile,
                                   const int *pxl, int pxl_cnt, int nMin, int nMax);

} // namespace internal
} // namespace libfa
