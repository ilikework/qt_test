#ifndef __LIB_FA_H__
#define __LIB_FA_H__

#ifdef LIBFA64_EXPORTS
#define LIBFA_API extern "C" __declspec(dllexport)
#else
#define LIBFA_API extern "C" __declspec(dllimport)
#endif

/* 原图属性 */
#define SOURCE_RGB          0
#define SOURCE_UV365        1
#define SOURCE_UV405        2
#define SOURCE_PL_POSITIVE  3
#define SOURCE_PL_NEGATIVE  4

/* 分析方法 */
#define METHOD_PORES        0
#define METHOD_SPOTS        1
#define METHOD_ACNES        2
#define METHOD_WRINKLES     3
#define METHOD_EVENNESS     4

typedef struct {
    int percent; /* 百分比 x10000 */
    int value;   /* 找到的区域数量 */
} T_ANA_RESULT;

typedef struct {
    int count;
    int *x;
    int *y;
} T_CONTOUR;

LIBFA_API bool initFaceDetector();
LIBFA_API void setExtraInfo(int age, int gender, int source_type);

LIBFA_API T_ANA_RESULT analysePoresByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax);
LIBFA_API T_ANA_RESULT analyseWrinkleByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax);
LIBFA_API T_ANA_RESULT analyseSpotsByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax);
LIBFA_API T_ANA_RESULT analyseAcnesByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax);
LIBFA_API T_ANA_RESULT analyseEvennessByFile(char *inFile, char *outFile, int *pxl, int pxl_cnt, int nMin, int nMax);

LIBFA_API T_CONTOUR autoMarkFaceByFile(char *fileName);
/* 侧脸 13 点。API 名与拍摄文件后缀一致，均使用 LeftFaceMask，检测路径对称：
 *   *_R.jpg -> autoMarkRightFaceByFile  原图方向 + 标准 landmark
 *   *_L.jpg -> autoMarkLeftFaceByFile   镜像检测 + 无 kIndex 重排 */
LIBFA_API T_CONTOUR autoMarkLeftFaceByFile(char *fileName);
LIBFA_API T_CONTOUR autoMarkRightFaceByFile(char *fileName);

LIBFA_API bool generateBrownSunburnPictureByFile(char *inFile, char *outFile);
LIBFA_API bool generateRedBloodPictureByFile(char *inFile, char *outFile);
LIBFA_API bool generateMixedSpotPictureByFile(char *inFile, char *outFile);

/* 释放 autoMarkFaceByFile 返回的轮廓内存
 * 坐标系：宽 768 逻辑像素，Y 自下而上（与 MagicFace 一致，非 OpenCV 左上角）
 * 映射到原图(OpenCV)：fullX = x*imageWidth/768
 *                    fullY = imageHeight-1 - y*imageWidth/768 */
LIBFA_API void freeContour(T_CONTOUR *contour);

/* demo: 读图自检，返回 cols*rows，失败返回 0 */
LIBFA_API int libfaTestReadImage(const char *path);

/* 与 autoMark* 内部 cv::imread 一致的原图像素尺寸，供坐标换算 */
LIBFA_API int libfaImageFileSize(const char *path, int *outW, int *outH);

#endif
