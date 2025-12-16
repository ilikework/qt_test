#pragma once
#include <objidl.h> 
#include <gdiplus.h>
#include <GdiPlusEnums.h> 
#include <string>
#include <vector>
#pragma comment(lib, "gdiplus.lib") 
using   namespace   Gdiplus;

class CGdiplusInit : public Gdiplus::GdiplusStartupOutput
{
public:
	CGdiplusInit(
		Gdiplus::DebugEventProc debugEventCallback = 0,
		BOOL                    suppressBackgroundThread = FALSE,
		BOOL                    suppressExternalCodecs = FALSE
	)
	{
		Gdiplus::GdiplusStartupInput StartupInput(
			debugEventCallback,
			suppressBackgroundThread,
			suppressExternalCodecs
		);

		StartupStatus = Gdiplus::GdiplusStartup(
			&Token,
			&StartupInput,
			this
		);
	}

	~CGdiplusInit()
	{
		if (StartupStatus == Gdiplus::Ok)
		{
			Gdiplus::GdiplusShutdown(Token);
		}
	}

	static int GetEncoderClsid(const std::wstring& format, CLSID* pClsid)
	{
		UINT num = 0;
		UINT size = 0;
		if (GetImageEncodersSize(&num, &size) != Ok || size == 0 || num == 0)
			return -1;

		// 用 vector 自动管理内存
		std::vector<BYTE> buffer(size);
		ImageCodecInfo* codecs = reinterpret_cast<ImageCodecInfo*>(buffer.data());

		if (GetImageEncoders(num, size, codecs) != Ok)
			return -1;

		for (UINT j = 0; j < num; ++j)
		{
			if (codecs[j].MimeType && std::wstring(codecs[j].MimeType) == format)
			{
				*pClsid = codecs[j].Clsid;
				return j; // 成功
			}
		}
		return -1; // 未找到
	}


private:
	Gdiplus::Status StartupStatus;
	ULONG_PTR       Token;
};