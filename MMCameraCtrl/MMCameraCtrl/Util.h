#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <filesystem>
#include <string>
#include <vector>
//#include "DBHandler.h"

enum {
    ILL_NONE_TYPE = 0,
    ILL_RGB_TYPE = 1,
    ILL_365UV_TYPE,
    ILL_405UV_TYPE,
    ILL_PL_TYPE,
    ILL_NPL_TYPE,
};

#define IDS_INIMAINFILENAME	L"Face.ini"

#define IDS_TEMP_DIR L"TempletePhoto\\"

struct ParamSetting {
    std::vector<std::wstring> options;  // 可选值，比如 {"100", "200", "400"}
    int defaultIndex;                   // 默认选中哪个值的索引
};

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

class Util
{

public:
    static Util& Instance();
    bool IsAutoCreate(int nILLType);

    int GetIntervalBeforeCapture(int nILLType);
    int GetIntervalAfterCapture(int nILLType);

    int GetAigoZoom();
    int GetCameraSeries();

    bool GetAigoFlash();

    std::wstring GetCameraConfigType(const std::wstring& type);
    std::wstring MappingJSCaptureTypeToDBColumn(const std::wstring& type);
    std::string WideToUtf8(const std::wstring& wstr);
    std::wstring Utf8ToWide(const std::string& str);

    std::wstring GetExeDir();

    std::wstring AnsiToWString(const char* str);
    std::string WStringToString(const std::wstring& wstr);

    void WriteLog(const std::wstring& message);


    std::string EncodeBase64Frame(const void* data, size_t len)
    {
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
        std::string out;
        out.reserve((len * 4 + 2) / 3 + len / 76);

        size_t i = 0;
        while (i < len) {
            unsigned int octet_a = i < len ? bytes[i++] : 0;
            unsigned int octet_b = i < len ? bytes[i++] : 0;
            unsigned int octet_c = i < len ? bytes[i++] : 0;

            unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

            out.push_back(base64_table[(triple >> 18) & 0x3F]);
            out.push_back(base64_table[(triple >> 12) & 0x3F]);
            out.push_back(i > len + 1 ? '=' : base64_table[(triple >> 6) & 0x3F]);
            out.push_back(i > len ? '=' : base64_table[triple & 0x3F]);
        }

        return out;
    }
    bool EnsureDirectory(const std::string& path)
    {
        try
        {
            if (std::filesystem::exists(path))
                return true;

            return std::filesystem::create_directories(path);
        }
        catch (...)
        {
            return false;
        }
    }
private:
    Util() = default;
    ~Util() = default;
    Util(const Util&) = delete;
    Util& operator=(const Util&) = delete;

    std::wstring GetTodayLogFileName();
};
