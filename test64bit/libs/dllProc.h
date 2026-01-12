#pragma once
// 紫外光图灰度化
_declspec(dllexport) int Gray_Enhance(wchar_t *pimg_in, wchar_t *pimg_out);

// 紫外光图获取棕色图(反应面部黑色素melanin情况)
_declspec(dllexport) int Extract_Melanin(wchar_t* pimg_in, wchar_t* pimg_out);

// 紫外光图获取红色图(反应面部毛细血管情况)
_declspec(dllexport) int Extract_Red_Skin(wchar_t* pimg_in, wchar_t* pimg_out);

// 根据偏光图增强面部斑点
_declspec(dllexport) int Enhance_Spots(wchar_t* pimg_in, wchar_t* pimg_out);

// 三张不同焦距的图像融合
_declspec(dllexport) int Three_Image_Fusion(wchar_t* pimg_in1, wchar_t* pimg_in2, wchar_t* pimg_in3, wchar_t* pimg_out);

// 两张不同焦距的图像融合
_declspec(dllexport) int Two_Image_Fusion(wchar_t* pimg_in1, wchar_t* pimg_in2, wchar_t* pimg_out);