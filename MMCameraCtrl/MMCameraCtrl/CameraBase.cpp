#include "CameraBase.h"
#include "USBControler.h"

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

long NullCamera::Capture(std::function<void()> callback)
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

void NullCamera::ReqOneFrame2(std::function<void(void*)> callback, void* param)
{
}

std::vector<uint8_t> NullCamera::GetFrame2()
{
    return std::vector<uint8_t>();
}
