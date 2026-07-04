#ifndef MM_CONST_DEFINE_H
#define MM_CONST_DEFINE_H

#define SLASH "/"
#define DIR_CUSTOMERS "customers"

#define JPG ".jpg"

#define LEFT "_L"
#define RIGHT "_R"
#define MM_RGB "RGB"
#define MM_UV "UV"
#define MM_PL "PL"
#define MM_NPL "NPL"
#define MM_GRAY "GRAY"
#define MM_RED "RED"
#define MM_BROWN "BROWN"
#define MM_WHOLE "WHOLE"

/// T_FacePhoto_AnalyseInfo / T_FacePhoto_Map.Analyse_Function（与 MagicFace M_*Type 一致）
#define MM_ANALYSE_SPOTS     1   /// AnalyseSpots — PL / NPL / BROWN
#define MM_ANALYSE_PORES     2   /// AnalysePores — RGB
#define MM_ANALYSE_EVENNESS  3   /// AnalyseEvenness — RED
#define MM_ANALYSE_WRINKLE   4   /// AnalyseWrinkle — GRAY
#define MM_ANALYSE_ACNES     5   /// AnalyseAcnes — UV
#define MM_ANALYSE_MOISTURE  21  /// Moisture — WHOLE（非 LibFA64）

static const char* DB_FILENAME = "MMFace_.db";



#endif // MM_CONST_DEFINE_H
