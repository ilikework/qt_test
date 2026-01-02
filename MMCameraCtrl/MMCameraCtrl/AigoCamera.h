#pragma once
#include "CameraBase.h"

#include <string>
#include <mutex>


typedef union
{
	ARGB Color;
	struct
	{
		BYTE Blue;
		BYTE Green;
		BYTE Red;
		BYTE Alpha;
	};
}ARGBQuad, * PARGBQuad;


class CAigoCamera : public ICameraBase
{
public:
	CAigoCamera();
	virtual ~CAigoCamera();

	virtual long Init(const wchar_t* pszDllName) override;
	virtual void unInit(bool bPowerOff = false) override;
	virtual bool StartPreview(void) override;
	virtual void StopPreview(void) override;
	virtual long Capture(uint32_t id, CapturedCallback callback) override;
	virtual void SetISO(int iso) override;
	virtual void SetWB(int wb) override;
	virtual void SetExposure(int exposure) override;
	virtual void SetZoom(int zoom) override;
	virtual void Autofocusing(int usauto) override;
	virtual void SetFocusType(int focusType) override;
	virtual void SetFlash(bool bflash) override;
	virtual void SetImgSize(int imgSize) override;
	virtual void SetImgQuality(int imgQuality) override;
	virtual void SetAperture(int aperture) override;
	virtual void SetFocusPos(int nPos) override;
	virtual void SetMode(int mode) override;
	virtual int GetMode() override { return 0; };


	virtual void PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc) override;
	void SetCaptureType(LONG nType) override;


	virtual bool IsPreviewStop() override { return m_bStop; }

	long InitFunctions(const wchar_t* pszDllName);

	static void CreateEdgePicture(const wchar_t* pszFrom, const wchar_t* pszTo, const RECT& rc, int nThreshold);
	void SetEdgeFileName(const wchar_t* pbstrFileName);
	void  LoadGif(const wchar_t* gifFileName);
	void DrawGif(Gdiplus::Graphics* g);
	int  GetGifFrameDelay();
	bool CreateFaceAngleAnalyseThread(int nFaceType);
	void StopFaceAngleAnalyse();
	HANDLE m_hFaceAngleAnalyseThreadHandle = NULL;
	int m_nFaceType = FACE_0_TYPE; // for face type middle , left , right.
	void SetFaceAngleAnalyseFileName(const wchar_t* szFaceAngleAnalyseFileName)
	{
		m_strFaceAngleAnalyseFileName = szFaceAngleAnalyseFileName;
	};


protected:
	static int m_nRef;
	static HMODULE		m_hModule;

	//CRITICAL_SECTION cs_locker;
	CGdiplusInit m_gdiplusInit;

	CRITICAL_SECTION m_gifLocker;
	Image* m_pGif = NULL;
	GUID m_guidDimension;
	UINT m_nFrameCount = 0;
	UINT m_nFrameIndex = 0;
	UINT m_nFrameDelay = 100; // 默认100ms

	CRITICAL_SECTION m_analyse_pic_locker;
	std::wstring m_strFaceAngleAnalyseFileName;

	void SaveFaceAngleAnalyseFile(Gdiplus::Bitmap* pImg);
	std::string GetSendCMD();
	std::wstring GetEXEName();
	PROCESS_INFORMATION   m_pi_face_angle_analyse;
	HANDLE m_hChildStd_IN_Rd = NULL;
	HANDLE m_hChildStd_IN_Wr = NULL;
	HANDLE m_hChildStd_OUT_Rd = NULL;
	HANDLE m_hChildStd_OUT_Wr = NULL;
	bool start_face_angle_analyse_progress();
	void end_face_angle_analyse_progress();
	int face_angle_analyse();
	void update_face_angle_analyse_result(int nResult);
	int m_face_angle_analyse_result = -1;
	bool		m_bAnalyseStop;
	static unsigned _stdcall FaceAngleAnalyseThread(LPVOID lpPara);
	std::wstring GetgifFileName(int face_angle_analyse_result);

	void Gray(BitmapData* data);
	void DrawWell(HDC hDC, int nWidth, int nHeight);
	void Draw3VLine(HDC hDC, int nWidth, int nHeight);
	void Draw3VLine(Gdiplus::Graphics* g, int nWidth, int nHeight);
	//virtual void DisplayBMPImage(unsigned char *pBmpData);
	static int GetPreviewRotateValue();
	static std::wstring GetIniFileName();
	//void CreatePreviewThread();
	//static unsigned _stdcall PreviewReadThreadBase(LPVOID lpPara);
	//void PreviewSaveBMP(BYTE * buf, UINT width, UINT height);
	//void YUVTORGB(LPVOID lpYUV, LPVOID lpBMP);
	static void do_analyse(int iHeight, int iWidth, unsigned char* p, unsigned char* p2, int stride, int nThreshold);
	static bool MirrorBmpImage(const wchar_t* inputPath, const wchar_t* outputPath);

	bool doCapture(const wchar_t* pstrFileName);
	static bool	m_bInit;
	LPBYTE	m_lpBuf1;	// 用于指向视频显示的两个缓冲区
	ULONG	m_Mode;
	USHORT	m_usVideo_w, m_usVideo_h;

	bool		m_bStop; // control preview thread end.
	bool		OneVideoFrameFinish;
	std::wstring m_strEdgFileName;


	int GetCameraSeries() override;

	std::string GetFrame() override;
	void ReqOneFrame(std::function<void()> callback) override;

	std::string BmpBufferToBase64Jpeg(LPBYTE pBmpBuf, int width, int height);

	std::mutex m_frameMutex;        // 互斥锁
	std::string m_oneframeBase64;// 当前可显示的帧


	// 通过 ICameraBase 继承
	bool GetSupportWBs(std::vector<int> &values) override;

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