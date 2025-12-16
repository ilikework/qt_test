#include "pch.h"
#include <cassert>
#include <format>
#include "GdiplusInit.h"
//#include "kzdsc.h"
#include "Util.h"
#include <fstream>
#include <ctime>
#include <mutex>
#include <sstream>
#include <iomanip>
//#include <fileapi.h>
//#include <WinBase.h>
//#include <WinNls.h>


std::mutex g_logMutex;

Util& Util::Instance()
{
    static Util instance;
    return instance;
}

bool Util::IsAutoCreate(int nILLType)
{
   if (nILLType== ILL_405UV_TYPE)
    {
       const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
       const std::wstring section = L"Camera";
       std::wstring key = L"AutoCreateUV405"; 
       if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
           return 0;

       // 读取整数值
       return GetPrivateProfileIntW(section.c_str(), key.c_str(), 1, iniFileName.c_str())==1 ? true:false;

    }
    return false;
}

int Util::GetIntervalBeforeCapture(int nILLType)
{
    const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
    const std::wstring section = L"Camera";
    std::wstring key = L"";
    switch (nILLType)
    {
        case ILL_RGB_TYPE:
            key = L"BeforeRGBShootInterval";
        break;
        case ILL_365UV_TYPE:
            key = L"BeforeUVShootInterval";
        break;
        case ILL_405UV_TYPE:
            key = L"Before405UVShootInterval";
        break;
        case ILL_PL_TYPE:
            key = L"BeforePLShootInterval";
        break;
        case ILL_NPL_TYPE:
            key = L"BeforeNPLShootInterval";
        break;
        default:
            return 0;
    }


    // 判断文件是否存在
    if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return 0;

    // 读取整数值
    return GetPrivateProfileIntW(section.c_str(),key.c_str(),0,iniFileName.c_str());
}

int Util::GetIntervalAfterCapture(int nILLType)
{
    const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
    const std::wstring section = L"Camera";
    std::wstring key = L"";
    switch (nILLType)
    {
    case ILL_RGB_TYPE:
        key = L"AfterRGBShootInterval";
        break;
    case ILL_365UV_TYPE:
        key = L"AfterUVShootInterval";
        break;
    case ILL_405UV_TYPE:
        key = L"After405UVShootInterval";
        break;
    case ILL_PL_TYPE:
        key = L"AfterPLShootInterval";
        break;
    case ILL_NPL_TYPE:
        key = L"AfterNPLShootInterval";
        break;
    default:
        return 0;
    }


    // 判断文件是否存在
    if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return 0;

    // 读取整数值
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), 0, iniFileName.c_str());
}

int Util::GetAigoZoom()
{
    const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
    const std::wstring section = L"DefaultCapInfoVanton";
    std::wstring key = L"strZoom";

    // 判断文件是否存在
    if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return 0;

    // 读取整数值
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), 0, iniFileName.c_str());
}

int Util::GetCameraSeries()
{
    const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
    const std::wstring section = L"Camera";
    std::wstring key = L"Series";

    // 判断文件是否存在
    if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return 0;

    // 读取整数值
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), 0, iniFileName.c_str());
}

bool Util::GetAigoFlash()
{
    const std::wstring iniFileName = GetExeDir() + L"\\" + IDS_INIMAINFILENAME;  // 替换为你的路径和文件名
    const std::wstring section = L"DefaultCapInfoVanton";
    std::wstring key = L"RGBFlash";

    // 判断文件是否存在
    if (GetFileAttributesW(iniFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return false;

    // 读取整数值
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), 0, iniFileName.c_str())==1 ? true:false;
}

std::wstring Util:: GetCameraConfigType(const std::wstring& type)
{
    if (   type == L"rgb_iso" 
        || type == L"uv_iso" 
        || type == L"pl_iso"
        || type == L"npl_iso")
    {
        return L"iso";
    }
    else if (  type == L"rgb_exposure"
            || type == L"uv_exposure"
            || type == L"pl_exposure"
            || type == L"npl_exposure")
    {
        return L"exposuretime";
    }
    else if (   type == L"rgb_aperture"
             || type == L"uv_aperture"
             || type == L"pl_aperture"
             || type == L"npl_aperture")
    {
        return L"aperture";
    }
    else if (   type == L"rgb_wb"
             || type == L"uv_wb"
             || type == L"pl_wb"
             || type == L"npl_wb")
    {
        return L"wb";
    }

    else if (type == L"image_size")
    {
        return L"ImageSize";
    }
    else if (type == L"image_quality")
    {
        return L"ImageQuality";
    }

    return L"";
}

std::wstring Util::MappingJSCaptureTypeToDBColumn(const std::wstring& type)
{
    if (type == L"rgb_iso")
    {
        return L"CAP_SceneISO";
    }
    else if (type == L"rgb_exposure")
    {
        return L"CAP_SceneExposureTime";
    }
    else if (type == L"rgb_aperture")
    {
        return L"CAP_SceneAperture";
    }
    else if (type == L"rgb_wb")
    {
        return L"CAP_WB";
    }
    else if (type == L"uv_iso")
    {
        return L"CAP_UVISO";
    }
    else if (type == L"uv_exposure")
    {
        return L"CAP_UVExposureTime";
    }
    else if (type == L"uv_aperture")
    {
        return L"CAP_UVAperture";
    }
    else if (type == L"uv_wb")
    {
        return L"CAP_WBUV";
    }
    else if (type == L"pl_iso")
    {
        return L"CAP_MCISO";
    }
    else if (type == L"pl_exposure")
    {
        return L"CAP_MCExposureTime";
    }
    else if (type == L"pl_aperture")
    {
        return L"CAP_MCAperture";
    }
    else if (type == L"pl_wb")
    {
        return L"CAP_MCWB";
    }
    else if (type == L"npl_iso")
    {
        return L"CAP_NEGMCISO";
    }
    else if (type == L"npl_exposure")
    {
        return L"CAP_NEGMCExposureTime";
    }
    else if (type == L"npl_aperture")
    {
        return L"CAP_NEGMCAperture";
    }
    else if (type == L"npl_wb")
    {
        return L"CAP_NEGMCWB";
    }
    else if (type == L"image_size")
    {
        return L"CAP_ImageSize";
    }
    else if (type == L"image_quality")
    {
        return L"CAP_ImageQuality";
    }

    return L"";
}

std::string Util::WideToUtf8(const std::wstring & wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), size - 1, nullptr, nullptr);
    return result;
}

std::wstring Util::Utf8ToWide(const std::string& str)
{
    if (str.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size - 1);
    return result;
}


std::wstring Util::GetExeDir()
{
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, path, MAX_PATH);

    std::wstring fullPath(path);
    size_t pos = fullPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        return fullPath.substr(0, pos);
    else
        return L"";  // fallback
}


std::wstring Util::AnsiToWString(const char* str)
{
    if (!str) return L"";

    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr.data(), len);

    // 移除末尾的 \0（因为构造时包含了它）
    if (!wstr.empty() && wstr.back() == L'\0')
        wstr.pop_back();

    return wstr;
}

std::string Util::WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return {};

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);

    return strTo;
}

std::wstring Util::GetTodayLogFileName()
{
    time_t now = time(nullptr);
    struct tm localTime;
    localtime_s(&localTime, &now);
 
    std::wstringstream ss;
    ss << GetExeDir() << L"\\" << L"log_" << std::put_time(&localTime, L"%Y-%m-%d") << L".txt";
    return ss.str();
}

void Util::WriteLog(const std::wstring& message)
{
    std::lock_guard<std::mutex> lock(g_logMutex);

    // 获取当前时间
    time_t now = time(nullptr);
    struct tm localTime;
    localtime_s(&localTime, &now);

    wchar_t timeStr[64];
    wcsftime(timeStr, sizeof(timeStr) / sizeof(wchar_t), L"%H:%M:%S", &localTime);

    std::wstring logFile = GetTodayLogFileName();

    // 写入日志
    std::wofstream ofs(logFile, std::ios::app);
    if (ofs)
    {
        ofs << L"[" << timeStr << L"] " << message << std::endl;
        ofs.close();
    }
}

