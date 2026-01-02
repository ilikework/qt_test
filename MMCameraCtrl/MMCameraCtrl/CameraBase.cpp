#include "CameraBase.h"
#include "USBControler.h"

void ICameraBase::BeforeCapture(int nILLTypes)
{
    switch (nILLTypes)
    {
    case ILL_RGB_TYPE:
        this->m_pCtrler->SetRGBModeOn();
        break;
    case ILL_365UV_TYPE:
        this->m_pCtrler->SetUVModeOn();
        break;
    case ILL_PL_TYPE:
        this->m_pCtrler->SetPLModeOn();
        break;
    case ILL_NPL_TYPE:
        this->m_pCtrler->SetNPLModeOn();
        break;
    }
}

void ICameraBase::AfterCapture(int nILLTypes)
{
    switch (nILLTypes)
    {
    case ILL_RGB_TYPE:
        this->m_pCtrler->SetRGBModeOff();
        break;
    case ILL_365UV_TYPE:
        this->m_pCtrler->SetUVModeOff();
        break;
    case ILL_PL_TYPE:
        this->m_pCtrler->SetPLModeOff();
        break;
    case ILL_NPL_TYPE:
        this->m_pCtrler->SetNPLModeOff();
        break;
    }
}


bool ICameraBase::SplitFile(const std::wstring& input, std::wstring& left, std::wstring& right)
{
    size_t dotPos = input.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
    {
        std::wstring baseName = input.substr(0, dotPos); // 去掉扩展名

        left = baseName + L"_L.jpg";
        right = baseName + L"_R.jpg";
        return CropAndSplitImage(input, left, right);
    }
    return false;
}

bool ICameraBase::CropAndSplitImage(const std::wstring& inputPath, const std::wstring& leftPath, const std::wstring& rightPath)
{
    // 启动 GDI+
    CGdiplusInit gdiplusInit;

    bool result = false;

    {
        Bitmap* bmp = Bitmap::FromFile(inputPath.c_str());
        if (!bmp || bmp->GetLastStatus() != Ok) {
            LOG(std::format(L"加载图片失败: {}", inputPath));
            delete bmp;
            return false;
        }

        UINT W = bmp->GetWidth();
        UINT H = bmp->GetHeight();

        // 计算目标竖版宽度
        UINT targetW = W;
        UINT targetH = W * 4 / 2 / 3;

        // 裁掉
        UINT x0 = 0;
        UINT y0 = 0;
        if (H > targetH)
            y0 = (H - targetH) / 2;

        // 创建裁剪后的 Bitmap
        Bitmap* cropped = new Bitmap(targetW, H, bmp->GetPixelFormat());
        {
            Graphics g(cropped);
            g.DrawImage(bmp, Rect(0, 0, targetW, targetH), x0, y0, targetW, targetH, UnitPixel);
        }

        // 切成两半
        UINT halfW = targetW / 2;
        Bitmap* leftBmp = new Bitmap(halfW, targetH, bmp->GetPixelFormat());
        Bitmap* rightBmp = new Bitmap(halfW, targetH, bmp->GetPixelFormat());

        {
            Graphics gLeft(leftBmp);
            gLeft.DrawImage(cropped, Rect(0, 0, halfW, targetH), 0, 0, halfW, targetH, UnitPixel);

            Graphics gRight(rightBmp);
            gRight.DrawImage(cropped, Rect(0, 0, halfW, targetH), halfW, 0, halfW, targetH, UnitPixel);
        }

        CLSID clsidJpg;
        CGdiplusInit::GetEncoderClsid(L"image/jpeg", &clsidJpg);

        // 保存两张图
        if (leftBmp->Save(leftPath.c_str(), &clsidJpg, nullptr) == Ok &&
            rightBmp->Save(rightPath.c_str(), &clsidJpg, nullptr) == Ok) {
            result = true;
        }

        delete bmp;
        delete cropped;
        delete leftBmp;
        delete rightBmp;
    }

    // 关闭 GDI+
    return result;
}


NullCamera::NullCamera()
{
    
}

long NullCamera::Init(const wchar_t* pszDllName)
{
    return 0;
}

void NullCamera::unInit(bool bPowerOff)
{
}

bool NullCamera::StartPreview(void)
{
    return false;
}

void NullCamera::ReqOneFrame(std::function<void()> callback)
{
}

std::string NullCamera::GetFrame()
{
    return std::string();
}

void NullCamera::StopPreview(void)
{
}

long NullCamera::Capture(uint32_t id, CapturedCallback callback)
{
    return 0;
}

void NullCamera::SetISO(int iso)
{
}

void NullCamera::SetWB(int wb)
{
}

void NullCamera::SetExposure(int exposure)
{
}

void NullCamera::SetZoom(int zoom)
{
}

void NullCamera::Autofocusing(int usauto)
{
}

void NullCamera::SetFocusType(int focusType)
{
}

void NullCamera::SetFlash(bool bflash)
{
}

void NullCamera::SetImgSize(int imgSize)
{
}

void NullCamera::SetImgQuality(int imgQuality)
{
}

void NullCamera::SetAperture(int aperture)
{
}

void NullCamera::SetFocusPos(int nPos)
{
}

void NullCamera::SetMode(int mode)
{
}

int NullCamera::GetMode()
{
    return 0;
}

bool NullCamera::IsPreviewStop()
{
    return false;
}

int NullCamera::GetCameraSeries()
{
    return NoneType;
}

void NullCamera::PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc)
{
}

void NullCamera::SetCaptureType(LONG nType)
{
}

bool NullCamera::GetSupportWBs(std::vector<int>& values)
{
    return false;
}

bool NullCamera::GetSupportISOs(std::vector<int>& values)
{
    return false;
}

bool NullCamera::GetSupportApertures(std::vector<int>& values)
{
    return false;
}

bool NullCamera::GetSupportExposures(std::vector<int>& values)
{
    return false;
}

void NullCamera::ReqOneFrame2()
{
}

std::vector<uint8_t> NullCamera::GetFrame2()
{
    return std::vector<uint8_t>();
}

bool NullCamera::IsInited()
{
    return false;
}

void NullCamera::PushGetEvent()
{
}
