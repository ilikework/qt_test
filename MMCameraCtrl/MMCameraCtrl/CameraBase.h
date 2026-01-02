#pragma once
#include "pch.h"
#include <windows.h>
#include <vector>
#include <string>
#include "./lib/dllProc.h"
#include "IControler.h"
#include "USBControler.h"
#include <filesystem>
#include <functional>

#pragma comment(lib,"./lib/dllProc.lib")


#define IDS_FACEANGLEANALYSEFILENAME	L"FaceAngleAnalyse.jpg"
#define IDS_FACEANGLEANALYSEEXENAME	L"FaceAngleReconAlg.exe"
#define IDS_GIF_LEFT	L"left.gif"
#define IDS_GIF_RIGHT	L"right.gif"
#define IDS_GIF_UP	L"up.gif"
#define IDS_GIF_NG	L"NG.gif"
#define IDS_GIF_OK	L"OK.gif"

// old 3 cameras defines , can delete later.
#define CAMERAMODE_NONE -1
#define CAMERAMODE_ONE 0
#define CAMERAMODE_M 1
#define CAMERAMODE_L 2
#define CAMERAMODE_R 3

#define CAMERAMODE_STR		_T("USB camera")
#define CAMERAMODE_STR_M	_T("Camera Z")
#define CAMERAMODE_STR_L	_T("Camera L")
#define CAMERAMODE_STR_R	_T("Camera R")



enum CameraSeries 
{
	NoneType =0,
	CanonGX = 1,
	CanonEOS = 4,            // add for 20140219
	Aigo = 5,     // add for 20110602
	TheSony = 7,           // add 20161111
};

struct Option
{
	std::wstring name;
	int value;
};

struct camera_param {
	int wb=0;
	int iso = 0;
	int aperture = 0;
	int exposure = 0;
};

class CaptureSetting {
private:
	int imgSize = 0;
	int imgQuality = 0; // use default.

	camera_param rgb;
	camera_param uv;
	camera_param pl;
	camera_param npl;


public:
	// --- General Getters/Setters ---
	int getImgSize() const { return imgSize; }
	void setImgSize(int val) { imgSize = val; }

	int getImgQuality() const { return imgQuality; }
	void setImgQuality(int val) { imgQuality = val; }

	camera_param get_rgb() const { return rgb; }
	void set_rgb(camera_param param) { rgb = param; }
	camera_param get_uv() const { return uv; }
	void set_uv(camera_param param) { uv = param; }
	camera_param get_pl() const { return pl; }
	void set_pl(camera_param param) { pl = param; }
	camera_param get_npl() const { return npl; }
	void set_npl(camera_param param) { npl = param; }
};

class CaptureSetting2 {
private:
	int m_imgSize = 0;
	int m_imgQuality = 0; // use default.

	camera_param m_param;
	std::string m_capture_type;


public:
	// --- General Getters/Setters ---
	int getImgSize() const { return m_imgSize; }
	void setImgSize(int val) { m_imgSize = val; }

	int getImgQuality() const { return m_imgQuality; }
	void setImgQuality(int val) { m_imgQuality = val; }

	std::string getCaptureType() const { return m_capture_type; }
	void setCaptureType(const std::string &capture_type) { m_capture_type = capture_type; }

	camera_param get_param() const { return m_param; }
	void set_param(camera_param param) { m_param = param; }
};


class CSAutoLock
{
public:
	CSAutoLock(CRITICAL_SECTION* pCS)
		: m_pCS(pCS)
	{
		EnterCriticalSection(m_pCS);
	}

	~CSAutoLock()
	{
		LeaveCriticalSection(m_pCS);
	}

private:
	CRITICAL_SECTION* m_pCS;
};

enum {
	FACE_0_TYPE = 0,
	FACE_L_TYPE,
	FACE_R_TYPE,
	FACE_M_TYPE
};

enum {
	PMode = 0,
	AMode = 1,
	SMode = 2,
	MMode = 3
};


class ICameraBase
{
public:
	ICameraBase() { }
	virtual ~ICameraBase() {}

	using CapturedCallback = std::function<void(const uint32_t id,const std::vector<std::string>& files)>;

	virtual long Init(const wchar_t* pszDllName) = 0;
	virtual void unInit(bool bPowerOff = false) = 0;
	virtual bool StartPreview(void) = 0;
	virtual void ReqOneFrame(std::function<void()> callback) = 0;
	virtual std::string GetFrame() = 0;
	virtual void ReqOneFrame2() = 0;
	virtual std::vector<uint8_t> GetFrame2() = 0;
	virtual void StopPreview(void) = 0;
	virtual long Capture(uint32_t id, CapturedCallback callback) = 0;
	virtual void SetZoom(int zoom) = 0;
	virtual void Autofocusing(int usauto) = 0;
	virtual void SetFocusType(int focusType) = 0;
	virtual void SetFlash(bool bflash) = 0;
	virtual void SetFocusPos(int nPos) = 0;
	virtual void SetMode(int mode) = 0;
	virtual int GetMode() = 0;
	virtual bool IsPreviewStop() = 0;
	virtual int GetCameraSeries() = 0;

	virtual void SetImgSize(int imgSize) = 0;
	virtual void SetImgQuality(int imgQuality) = 0;
	virtual void SetISO(int iso) = 0;
	virtual void SetWB(int wb) = 0;
	virtual void SetExposure(int exposure) = 0;
	virtual void SetAperture(int aperture) = 0;

	virtual bool GetSupportWBs(std::vector<int>& values) = 0;
	virtual bool GetSupportISOs(std::vector<int>& values) = 0;
	virtual bool GetSupportApertures(std::vector<int>& values) = 0;
	virtual bool GetSupportExposures(std::vector<int>& values) = 0;

	virtual void PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc) = 0;
	virtual void SetCaptureType(LONG nType) = 0;
	virtual bool IsInited() = 0;
	virtual void PushGetEvent() = 0; // special for Canon EDS

	void setCaptureSetting(const CaptureSetting& captureSetting)
	{
		m_captureSetting = captureSetting;
	}

	CaptureSetting getCaptureSetting() const
	{
		return m_captureSetting;
	}

	void SetCaptureFolder(const std::string & folderPath)
	{
		Util::Instance().EnsureDirectory(folderPath);
		
		m_folderPath = Util::Instance().AnsiToWString(folderPath.c_str());
	}

	void SetCaptureID(const int nID)
	{
		m_nID = nID;
	}

	std::wstring GetCaptureFileName(int nIllType)
	{
		std::wstring str = m_folderPath;
		str  += L"/";
		str += std::format(L"{:02}", m_nID);
		str += L".jpg";
		return str;
	}

	void SetILLType(int nILLType) { m_nILLType = nILLType; }
	int  GetILLType() { return m_nILLType; }
	void BeforeCapture(int nILLTypes);
	void AfterCapture(int nILLTypes);
	bool CropAndSplitImage(const std::wstring& inputPath, const std::wstring& leftPath, const std::wstring& rightPath);
	bool SplitFile(const std::wstring& input, std::wstring& left, std::wstring& right);

	
	void PausePreview()
	{
		m_bPause = true;
	}

	void ResumePreview()
	{
		m_bPause = false;
	}
	bool IsPausePreview()
	{
		return m_bPause;
	}

protected:
	std::wstring m_folderPath = L"";
	int m_nID = 0;
	int m_nILLType = ILL_NONE_TYPE;
	std::unique_ptr <IControler> m_pCtrler = std::make_unique<USBControler>();
	CaptureSetting m_captureSetting;

	std::atomic<bool> m_bPause{ true };


};

class NullCamera :public ICameraBase
{

public:
	NullCamera();

	// 通过 ICameraBase 继承
	long Init(const wchar_t* pszDllName) override;
	void unInit(bool bPowerOff) override;
	bool StartPreview(void) override;
	void ReqOneFrame(std::function<void()> callback) override;
	std::string GetFrame() override;
	void StopPreview(void) override;
	long Capture(uint32_t id, CapturedCallback callback) override;
	void SetISO(int iso) override;
	void SetWB(int wb) override;
	void SetExposure(int exposure) override;
	void SetZoom(int zoom) override;
	void Autofocusing(int usauto) override;
	void SetFocusType(int focusType) override;
	void SetFlash(bool bflash) override;
	void SetImgSize(int imgSize) override;
	void SetImgQuality(int imgQuality) override;
	void SetAperture(int aperture) override;
	void SetFocusPos(int nPos) override;
	void SetMode(int mode) override;
	int GetMode() override;
	bool IsPreviewStop() override;
	int GetCameraSeries() override;
	void PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc) override;
	void SetCaptureType(LONG nType) override;

	// 通过 ICameraBase 继承
	bool GetSupportWBs(std::vector<int>& values) override;
	bool GetSupportISOs(std::vector<int>& values) override;
	bool GetSupportApertures(std::vector<int>& values) override;
	bool GetSupportExposures(std::vector<int>& values) override;

	// 通过 ICameraBase 继承
	void ReqOneFrame2() override;
	std::vector<uint8_t> GetFrame2() override;

	// 通过 ICameraBase 继承
	bool IsInited() override;

	// 通过 ICameraBase 继承
	void PushGetEvent() override;
};
