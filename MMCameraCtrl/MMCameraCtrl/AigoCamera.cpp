#include "pch.h"
#include "AigoCamera.h"
#include "kzdsc.h"
#include <cctype>
#include <thread>
#include <Shlwapi.h>
#include <filesystem>
#include <sstream>
#include <atlbase.h>  // CComPtr
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#define UNDEF			0xffffffff
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)


bool CAigoCamera::m_bInit = false;
HMODULE CAigoCamera::m_hModule = NULL;

CAigoCamera::CAigoCamera()
	:m_bStop(true),  m_lpBuf1(NULL),
	OneVideoFrameFinish(true), m_Mode(0), m_usVideo_w(0), m_usVideo_h(0),
	m_bAnalyseStop(true) {

	InitializeCriticalSection(&m_analyse_pic_locker);
	InitializeCriticalSection(&m_gifLocker);
	memset(&m_pi_face_angle_analyse, 0x00, sizeof(m_pi_face_angle_analyse));
	memset(&m_guidDimension, 0, sizeof(m_guidDimension));

}


CAigoCamera::~CAigoCamera()
{
	DeleteCriticalSection(&m_analyse_pic_locker);
	DeleteCriticalSection(&m_gifLocker);
}


long CAigoCamera::Init(const wchar_t* pszDllName)
{
	LOG(L"init in m_bInit = " + std::to_wstring(m_bInit));
	if (m_bInit)
		return 0;
	
	if (this->InitFunctions(pszDllName) != 0)
	{
		LOG(L"not find kzdsc.dll");
		return -1;
	}

	if (!InitHardWare())
	{
		//theApp.GetMainWnd()->MessageBox(_T("inital failed, continue."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"inital failed, continue.");
		return -1;
	}

	if (!VTIsSDKCorrect())
	{
		//theApp.GetMainWnd()->MessageBox(_T("SDK version error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"SDK version error.");
		return -1;
	}

	if (!KZDisablePowerSave())
	{
		//theApp.GetMainWnd()->MessageBox(_T("PowerSave setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"PowerSave setting error.");
		return -1;
	}

	if (!KZInitDSCParam())
	{
		//theApp.GetMainWnd()->MessageBox(_T("Param setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"Param setting error.");
		return -1;
	}

	if (!KZFlashModeSet(4))
	{
		//theApp.GetMainWnd()->MessageBox(_T("FlashMode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"FlashMode setting error.");
		return -1;
	}

	if (!KZEnterPMode())
	{
		//theApp.GetMainWnd()->MessageBox(_T("P Mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"P Mode setting error.");
		return -1;
	}
	m_Mode = PMode; // set to PMode 20141124
	Sleep(3000);	//wait P mode change;

	if (!KZGetVideoSize(&m_usVideo_w, &m_usVideo_h))
	{
		//theApp.GetMainWnd()->MessageBox(_T("read Video error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"read Video error.");
		return -1;
	}

	LOG(std::format(L"m_usVideo_w={}, m_usVideo_h={}", m_usVideo_w, m_usVideo_h));
	if (!KZPreviewStartVideo())
	{
		//theApp.GetMainWnd()->MessageBox(_T("Preview start Video error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"Preview start Video error.");
		return -1;
	}

	if (!KZSetUploadMode())
	{
		//theApp.GetMainWnd()->MessageBox(_T("upload mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"upload mode setting error.");
		return -1;
	}
	m_bInit = true;
	return 0;
}

long CAigoCamera::InitFunctions(const wchar_t* pszDllName)
{
	if (m_hModule)
		return 0;
	m_hModule = LoadLibrary(pszDllName);
	if (m_hModule)
	{
		InitHardWare = (pInitHardWare)GetProcAddress(m_hModule, "InitHardWare");
		UnintHWare = (pUnintHWare)GetProcAddress(m_hModule, "UnintHWare");
		VTIsSDKCorrect = (pVTIsSDKCorrect)GetProcAddress(m_hModule, "VTIsSDKCorrect");
		KZGetVideoSize = (pKZGetVideoSize)GetProcAddress(m_hModule, "KZGetVideoSize");
		KZPreviewStartVideo = (pKZPreviewStartVideo)GetProcAddress(m_hModule, "KZPreviewStartVideo");
		KZPreviewOneVideoFrameRGB = (pKZPreviewOneVideoFrameRGB)GetProcAddress(m_hModule, "KZPreviewOneVideoFrameRGB");
		KZPreviewStopVideo = (pKZPreviewStopVideo)GetProcAddress(m_hModule, "KZPreviewStopVideo");
		KZSetUploadMode = (pKZSetUploadMode)GetProcAddress(m_hModule, "KZSetUploadMode");
		KZSetNormalStoreMode = (pKZSetNormalStoreMode)GetProcAddress(m_hModule, "KZSetNormalStoreMode");
		KZUpload = (pKZUpload)GetProcAddress(m_hModule, "KZUpload");
		KZCapture = (pKZCapture)GetProcAddress(m_hModule, "KZCapture");
		KZFlashModeSet = (pKZFlashModeSet)GetProcAddress(m_hModule, "KZFlashModeSet");
		KZFireOutsideFlash = (pKZFireOutsideFlash)GetProcAddress(m_hModule, "KZFireOutsideFlash");
		KZFirePower_OutsideFlash = (pKZFirePower_OutsideFlash)GetProcAddress(m_hModule, "KZFirePower_OutsideFlash");
		KZIsUseCaptureAF = (pKZIsUseCaptureAF)GetProcAddress(m_hModule, "KZIsUseCaptureAF");
		KZAFMode = (pKZAFMode)GetProcAddress(m_hModule, "KZAFMode");
		KZFlashCharge = (pKZFlashCharge)GetProcAddress(m_hModule, "KZFlashCharge");
		KZEnterPMode = (pKZEnterPMode)GetProcAddress(m_hModule, "KZEnterPMode");
		KZEnterAMode = (pKZEnterAMode)GetProcAddress(m_hModule, "KZEnterAMode");
		KZEnterSMode = (pKZEnterSMode)GetProcAddress(m_hModule, "KZEnterSMode");
		KZEnterMMode = (pKZEnterMMode)GetProcAddress(m_hModule, "KZEnterMMode");

		KZASM_AVSET = (pKZASM_AVSET)GetProcAddress(m_hModule, "KZASM_AVSET");
		KZASM_TVSET = (pKZASM_TVSET)GetProcAddress(m_hModule, "KZASM_TVSET");
		KZDisablePowerSave = (pKZDisablePowerSave)GetProcAddress(m_hModule, "KZDisablePowerSave");
		KZEnablePowerSave = (pKZEnablePowerSave)GetProcAddress(m_hModule, "KZEnablePowerSave");

		KZLcdOn = (pKZLcdOn)GetProcAddress(m_hModule, "KZLcdOn");
		KZLcdOff = (pKZLcdOff)GetProcAddress(m_hModule, "KZLcdOff");
		KZKEYOn = (pKZKEYOn)GetProcAddress(m_hModule, "KZKEYOn");
		KZKEYOff = (pKZKEYOff)GetProcAddress(m_hModule, "KZKEYOff");
		KZLensClose = (pKZLensClose)GetProcAddress(m_hModule, "KZLensClose");
		KZLensOut = (pKZLensOut)GetProcAddress(m_hModule, "KZLensOut");
		KZPowerOff = (pKZPowerOff)GetProcAddress(m_hModule, "KZPowerOff");
		KZGetZoomPos = (pKZGetZoomPos)GetProcAddress(m_hModule, "KZGetZoomPos");
		KZZoom = (pKZZoom)GetProcAddress(m_hModule, "KZZoom");
		LOG(std::format(L"init KZZoom = {}", (DWORD)KZZoom));
		KZGetFocusPos = (pKZGetFocusPos)GetProcAddress(m_hModule, "KZGetFocusPos");
		KZFocusPos = (pKZFocusPos)GetProcAddress(m_hModule, "KZFocusPos");
		KZIsLensMoving = (pKZIsLensMoving)GetProcAddress(m_hModule, "KZIsLensMoving");
		KZIsFlashCharging = (pKZIsFlashCharging)GetProcAddress(m_hModule, "KZIsFlashCharging");
		KZImageSizeSet = (pKZImageSizeSet)GetProcAddress(m_hModule, "KZImageSizeSet");
		KZSetQuality = (pKZSetQuality)GetProcAddress(m_hModule, "KZSetQuality");
		KZBiasSet = (pKZBiasSet)GetProcAddress(m_hModule, "KZBiasSet");
		KZAWBMode = (pKZAWBMode)GetProcAddress(m_hModule, "KZAWBMode");
		KZISOSet = (pKZISOSet)GetProcAddress(m_hModule, "KZISOSet");
		KZMeterMode = (pKZMeterMode)GetProcAddress(m_hModule, "KZMeterMode");
		KZSharpMode = (pKZSharpMode)GetProcAddress(m_hModule, "KZSharpMode");

		KZModify_para = (pKZModify_para)GetProcAddress(m_hModule, "KZModify_para");
		KZIdle = (pKZIdle)GetProcAddress(m_hModule, "KZIdle");
		KZPreview = (pKZPreview)GetProcAddress(m_hModule, "KZPreview");

		KZInitDSCParam = (pKZInitDSCParam)GetProcAddress(m_hModule, "KZInitDSCParam");
		KZAFAreaSet = (pKZAFAreaSet)GetProcAddress(m_hModule, "KZAFAreaSet");
		KZAF = (pKZAF)GetProcAddress(m_hModule, "KZAF");
		KZIsDoingAF = (pKZIsDoingAF)GetProcAddress(m_hModule, "KZIsDoingAF");
		KZSwitchOSDOnOff = (pKZSwitchOSDOnOff)GetProcAddress(m_hModule, "KZSwitchOSDOnOff");
		KZAWBCustomRGB = (pKZAWBCustomRGB)GetProcAddress(m_hModule, "KZAWBCustomRGB");
		KZVersion = (pKZVersion)GetProcAddress(m_hModule, "KZVersion");
	}
	else
	{
		return -1;
	}

	return 0;
}

void CAigoCamera::unInit(bool bPowerOff)
{
	LOG(std::format(L"unInit start m_bInit={}, bPowerOff={}", m_bInit, bPowerOff));
	if (m_bInit)
	{
		if (!KZPreviewStopVideo())
			//theApp.GetMainWnd()->MessageBox(_T("PreviewStopVideo Set failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		    LOG(L"PreviewStopVideo Set failed.");

		if (bPowerOff)
		{
			if (!KZPowerOff())
				//theApp.GetMainWnd()->MessageBox(_T("KZPowerOff failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
				LOG(L"KZPowerOff failed.");
		}
		if (m_hModule)
		{
			UnintHWare();
			FreeLibrary(m_hModule);
			m_hModule = NULL;
		}
	}
}

bool CAigoCamera::StartPreview(void)
{
	if (m_bStop == false)
		return false;

	this->ResumePreview();
	if (!m_lpBuf1)
	{
		m_lpBuf1 = new BYTE[m_usVideo_w*m_usVideo_h * 3];
		if (!m_lpBuf1)
			//theApp.GetMainWnd()->MessageBox(_T("Create m_lpBuf1 error"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"Create m_lpBuf1 error.");
	}
	KZIsUseCaptureAF(0);
	//if (Util::Instance().GetCaptureSetting(Aigo, m_captureSetting))
	{
		this->SetZoom(Util::Instance().GetAigoZoom());
		this->SetWB(m_captureSetting.get_rgb().wb);
		this->SetISO(m_captureSetting.get_rgb().iso);
		this->SetAperture(m_captureSetting.get_rgb().aperture);
		this->SetExposure(m_captureSetting.get_rgb().exposure);
		this->SetImgQuality(m_captureSetting.getImgQuality());
		this->SetImgSize(m_captureSetting.getImgSize());
		this->SetFlash(Util::Instance().GetAigoFlash());

		this->SetFocusType(1); // 设定近焦
		this->Autofocusing(1); // 设定自动对焦

		//CreatePreviewThread();
		m_bStop = false;
	}
	

	return true;
}

//void CAigoCamera::CreatePreviewThread()
//{
//	TRACE(L"CAigoCamera::CreatePreviewThread\n");
//	m_bStop = false;
//	unsigned threadid;
//	// 创建读视频流线程
//	HANDLE hReadThread1 = (HANDLE)_beginthreadex(NULL, 0,
//		PreviewReadThreadBase, (void*)this, CREATE_SUSPENDED, &threadid);
//	if (hReadThread1)
//	{
//		SetThreadPriority(hReadThread1, THREAD_PRIORITY_HIGHEST);
//		ResumeThread(hReadThread1);
//	}
//
//}

void CAigoCamera::StopPreview(void)
{
	// TODO: Add your dispatch handler code here
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bStop = true;

	Sleep(100);
	if (m_lpBuf1)
	{
		delete[]m_lpBuf1;
		m_lpBuf1 = NULL;
	}
	m_bPause = true;

	//???? why not use stop???
	//if(!KZPreviewStopVideo())
	//	AfxMessageBox(_T("PreviewStopVideo Set failed"));		
}

// 读视频数据线程函数
//unsigned _stdcall CAigoCamera::PreviewReadThreadBase(LPVOID lpPara)
//{
//	BOOL bRet;
//	CAigoCamera* pSelf = (CAigoCamera*)lpPara;
//
//	while (!pSelf->m_bStop)
//	{
//		if (!pSelf->m_bPause) {
//			pSelf->OneVideoFrameFinish = false;
//			pSelf->Lock(&pSelf->cs_locker);
//
//			bRet = KZPreviewOneVideoFrameRGB(pSelf->m_lpBuf1);
//			pSelf->Unlock(&pSelf->cs_locker);
//			pSelf->OneVideoFrameFinish = true;
//			//SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
//		}
//		Sleep(15);
//	}
//	pSelf->OneVideoFrameFinish = true;
//	return 0;
//}

int CAigoCamera::GetPreviewRotateValue()
{
	const std::wstring sectionName = L"DefaultCapInfoUSBCamera";
	const std::wstring iniFileName = GetIniFileName(); // 假设返回 std::wstring

	const std::wstring itemName = L"PreviewRotate";

	// GetPrivateProfileInt 要用 LPCWSTR，所以传 c_str()
	int nTmp = ::GetPrivateProfileIntW(
		sectionName.c_str(),
		itemName.c_str(),
		0,
		iniFileName.c_str()
	);
	return nTmp;
}

std::wstring CAigoCamera::GetIniFileName()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(NULL, path, MAX_PATH);

	std::wstring str(path);
	auto pos = str.find_last_of(L'\\');
	if (pos != std::wstring::npos)
		str.resize(pos + 1);

	str += IDS_INIMAINFILENAME;  // 直接写ini文件名

	return str;
}

void CAigoCamera::SaveFaceAngleAnalyseFile(Gdiplus::Bitmap* pImg)
{
	CSAutoLock lock(&m_analyse_pic_locker);

	if (m_strFaceAngleAnalyseFileName.empty())
		return;


	CLSID jpgClsid;
	CGdiplusInit::GetEncoderClsid(L"image/jpeg", &jpgClsid);

	pImg->Save(m_strFaceAngleAnalyseFileName.c_str(), &jpgClsid, NULL);
}

bool CAigoCamera::CreateFaceAngleAnalyseThread(int nFaceType)
{
	m_nFaceType = nFaceType;
	if (m_hFaceAngleAnalyseThreadHandle != NULL)
		return 0;
	LOG(L"CAigoCamera::CreateFaceAngleAnalyseThread");
	m_bAnalyseStop = false;
	unsigned threadid;
	// 创建读视频流线程
	m_hFaceAngleAnalyseThreadHandle = (HANDLE)_beginthreadex(NULL, 0,
		FaceAngleAnalyseThread, (void*)this, CREATE_SUSPENDED, &threadid);

	if (m_hFaceAngleAnalyseThreadHandle != NULL)
	{
		SetThreadPriority(m_hFaceAngleAnalyseThreadHandle, THREAD_PRIORITY_HIGHEST);
		ResumeThread(m_hFaceAngleAnalyseThreadHandle);
	}

	return false;
}

void CAigoCamera::StopFaceAngleAnalyse()
{
	if (m_hFaceAngleAnalyseThreadHandle)
	{
		m_bAnalyseStop = true;
		WaitForSingleObject(m_hFaceAngleAnalyseThreadHandle, INFINITE);
		CloseHandle(m_hFaceAngleAnalyseThreadHandle);

		m_hFaceAngleAnalyseThreadHandle = NULL;
		LoadGif(L""); // clear gif.
		m_face_angle_analyse_result = -1;
	}
}

unsigned _stdcall CAigoCamera::FaceAngleAnalyseThread(LPVOID lpPara)
{
	CAigoCamera* p = ((CAigoCamera*)lpPara);

	p->start_face_angle_analyse_progress();
	while (!p->m_bAnalyseStop)
	{
		int nResult = p->face_angle_analyse();
		if(nResult>=0)
			p->update_face_angle_analyse_result(nResult);
		Sleep(100);

	}
	p->end_face_angle_analyse_progress();

	LOG(L"FaceAngleAnalyseThread is end.");

	return 0;
}


std::string CAigoCamera::GetSendCMD()
{
	std::string str = Util::Instance().WStringToString(m_strFaceAngleAnalyseFileName) + " ";
	switch (m_nFaceType)
	{
	case FACE_L_TYPE:
		str += "0\r\n";
		break;
	case FACE_R_TYPE:
		str += "1\r\n";
		break;
	default:
		return "";
	}
	return str;
}

std::wstring CAigoCamera::GetEXEName()
{
	const std::wstring sectionName = L"Application";
	const std::wstring itemName = L"3DEXE";
	const std::wstring iniFileName = GetIniFileName();  // 返回 std::wstring

	wchar_t szWorkDir[MAX_PATH + 1] = {};

	DWORD dwReaded = ::GetPrivateProfileStringW(
		sectionName.c_str(),
		itemName.c_str(),
		L"",
		szWorkDir,
		MAX_PATH,
		iniFileName.c_str()
	);

	if (dwReaded == 0)
	{
		// 读取失败，回退当前模块路径
		GetModuleFileNameW(NULL, szWorkDir, MAX_PATH);
		PathRemoveFileSpecW(szWorkDir); // 去除 exe 文件名
		wcscat_s(szWorkDir, L"\\");     // 添加斜杠
	}
	else
	{
		// 读取到了路径，去掉最后两级目录
		PathRemoveFileSpecW(szWorkDir); // 去除 exe 文件名
		PathRemoveFileSpecW(szWorkDir); // 再去除一级子目录
	}

	// 构建最终路径
	std::wstring path(szWorkDir);
	path += L"3D\\FaceAngleReconAlg\\";
	path += IDS_FACEANGLEANALYSEEXENAME;  // 从资源读取 EXE 文件名

	return path;
}

std::wstring CAigoCamera::GetgifFileName(int face_angle_analyse_result)
{
	wchar_t szWorkDir[MAX_PATH + 1] = {};
	GetModuleFileNameW(NULL, szWorkDir, MAX_PATH);

	PathRemoveFileSpecW(szWorkDir);  // 去掉 exe 名称

	std::wstring path(szWorkDir);
	path += L"\\PresetImg\\";

	std::wstring strAdd = L"";
	switch (face_angle_analyse_result)
	{
	case 0: strAdd = IDS_GIF_NG; break;
	case 1: strAdd = IDS_GIF_LEFT; break;
	case 2: strAdd = IDS_GIF_RIGHT; break;
	case 3: strAdd = IDS_GIF_UP; break;
	case 4: strAdd = IDS_GIF_OK; break;
	default: return L"";  // 无效值
	}

	path += strAdd;
	return path;
}

bool CAigoCamera::start_face_angle_analyse_progress()
{
	bool ret = false;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
	{
		LOG(L"CreatePipe m_hChildStd_OUT_Rd Error");
		end_face_angle_analyse_progress();
		return ret;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		LOG(L"SetHandleInformation m_hChildStd_OUT_Rd Error");
		end_face_angle_analyse_progress();
		return ret;
	}

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))
	{
		LOG(L"CreatePipe m_hChildStd_IN_Rd Error");
		end_face_angle_analyse_progress();
		return ret;
	}

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
	{
		LOG(L"SetHandleInformation m_hChildStd_IN_Wr Error");
		end_face_angle_analyse_progress();
		return ret;
	}

	STARTUPINFO          si;
	memset(&si, 0x00, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = m_hChildStd_OUT_Wr;// GetStdHandle(STD_ERROR_HANDLE);
	si.hStdOutput = m_hChildStd_OUT_Wr;// GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdInput = m_hChildStd_IN_Rd;
	si.dwFlags |= STARTF_USESTDHANDLES;
	memset(&m_pi_face_angle_analyse, 0x00, sizeof(m_pi_face_angle_analyse));

	//_putenv("EURPS_REPORTPATH=.");
	//_putenv("EURPS_MAPDATAPATH=.");
	//_putenv("EURPS_OUTPUTPATH=.");

	std::wstring szCmdLine;
	szCmdLine = GetEXEName();
	BOOL bRtn = CreateProcess(NULL,        //ファイル名
		szCmdLine.data(),     //ファイル名+コマンド構文
		NULL,        //プロセス・セキュリティ
		NULL,        //スレッド・セキュリティ
		TRUE,        //ハンドルを継承するか？
		DETACHED_PROCESS,      //生成フラグ
		NULL,        //継承される環境ブロック
		NULL,        //起動時のフォルダ
		&si,         //スタートアップ情報へのポインタ
		&m_pi_face_angle_analyse);        //プロセス情報へのポインタ
	if (!bRtn) {
		LOG(L"CreateProcess Error");
		end_face_angle_analyse_progress();
		return ret;
	}

	char sbuf[4096];
	DWORD dw = 0;
	memset(sbuf, 0x00, sizeof(sbuf));
	if (!ReadFile(m_hChildStd_OUT_Rd, sbuf, 4095, &dw, NULL)) {
		DWORD dwcode = GetLastError();
		/* BROKEN_PIPEはエラーではない */
		if (dwcode != ERROR_BROKEN_PIPE) {
			end_face_angle_analyse_progress();
			return ret;
		}
	}
	sbuf[4095] = '\0';
	if (dw > 0 && strcmp(sbuf, "init\r\n") == 0)
	{
		LOG(L"init done");
	}
	else
	{
		end_face_angle_analyse_progress();
		return ret;
	}
	Sleep(100); // FaceAngleReconAlg is slow.

	if (!ReadFile(m_hChildStd_OUT_Rd, sbuf, 4095, &dw, NULL)) {
		DWORD dwcode = GetLastError();
		/* BROKEN_PIPEはエラーではない */
		if (dwcode != ERROR_BROKEN_PIPE) {
			end_face_angle_analyse_progress();
			return ret;
		}
	}
	if (dw > 0 && strncmp(sbuf, "INFO",4) == 0)
	{
		LOG(L"FaceAngleReconAlg.exe return is: " + Util::Instance().AnsiToWString(sbuf));
	}


	return true;
}

int CAigoCamera::face_angle_analyse()
{
	CSAutoLock lock(&m_analyse_pic_locker);

	int ret = -1;
	if (!PathFileExists(this->m_strFaceAngleAnalyseFileName.c_str()))
	{
		LOG(L"not exist FaceAngleAnalyseFile");
		Sleep(100);
		return ret;
	}

	char sbuf[4096];
	DWORD dw = 0;
	memset(sbuf, 0x00, sizeof(sbuf));

	const std::string& strSend = GetSendCMD();
	if (!m_pi_face_angle_analyse.hProcess)
	{
		LOG(L"Process is not initaled!");
		return ret;
	}
	if (strSend.size()<=0)
	{
		LOG(L"send command is empty!");
		return ret;
	}

	LOG(L"send FaceAngleReconAlg.exe: " + Util::Instance().AnsiToWString(strSend.c_str()));
	if (!WriteFile(m_hChildStd_IN_Wr, strSend.c_str(), strSend.length(), &dw, NULL)) {
		DWORD dwcode = GetLastError();
		LOG(std::format(L"FaceAngleReconAlg.exe write error , code is {} ", dwcode));

		/* BROKEN_PIPEはエラーではない */
		if (dwcode != ERROR_BROKEN_PIPE) {
			end_face_angle_analyse_progress();
			return ret;
		}
	}

	if (!ReadFile(m_hChildStd_OUT_Rd, sbuf, 4095, &dw, NULL)) {
		DWORD dwcode = GetLastError();
		LOG(std::format(L"FaceAngleReconAlg.exe read error , code is {} ", dwcode));
		/* BROKEN_PIPEはエラーではない */
		if (dwcode != ERROR_BROKEN_PIPE) {
			end_face_angle_analyse_progress();
			return ret;
		}
	}

	LOG(L"FaceAngleReconAlg.exe return is: " + Util::Instance().AnsiToWString(sbuf));
	if (dw > 0 && std::isdigit(sbuf[0]))
	{
		ret = sbuf[0] - '0';
	}

	return ret;
}

void CAigoCamera::end_face_angle_analyse_progress()
{
	LOG(L"CAigoCamera::end_face_angle_analyse_progress start");
	DWORD dw = 0;
	if (m_pi_face_angle_analyse.hProcess)
	{
		std::string send = "exit\r\n";
		WriteFile(m_hChildStd_IN_Wr, send.c_str(), send.length(), &dw, NULL);
		LOG(L"FaceAngleReconAlg.exe send exit");
	}

	if (m_pi_face_angle_analyse.hThread)
	{
		CloseHandle(m_pi_face_angle_analyse.hThread);
	}


	if (m_hChildStd_IN_Rd)
	{
		CloseHandle(m_hChildStd_IN_Rd);
		m_hChildStd_IN_Rd = NULL;
	}


	if (m_hChildStd_IN_Wr)
	{
		CloseHandle(m_hChildStd_IN_Wr);
		m_hChildStd_IN_Wr = NULL;
	}

	if (m_hChildStd_OUT_Rd)
	{
		CloseHandle(m_hChildStd_OUT_Rd);
		m_hChildStd_OUT_Rd = NULL;
	}

	if (m_hChildStd_OUT_Wr)
	{
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}

	if (m_pi_face_angle_analyse.hProcess)
	{
		BOOL result = CloseHandle(m_pi_face_angle_analyse.hProcess);
		if (!result)
		{
			DWORD err = GetLastError();
			wprintf(L"CloseHandle failed, error code: %lu\n", err);

			// 判断是否有需要 TerminateProcess 的场景
			// 比如程序逻辑中发现子进程仍在运行，且必须杀掉
			DWORD exitCode = 0;
			if (GetExitCodeProcess(m_pi_face_angle_analyse.hProcess, &exitCode))
			{
				if (exitCode == STILL_ACTIVE)
				{
					// 进程还活着，你才考虑 TerminateProcess
					TerminateProcess(m_pi_face_angle_analyse.hProcess, 0);
					CloseHandle(m_pi_face_angle_analyse.hProcess);
				}
			}

		}
	}


	memset(&m_pi_face_angle_analyse, 0x00, sizeof(m_pi_face_angle_analyse));
}

void CAigoCamera::update_face_angle_analyse_result(int nResult)
{
	if (m_face_angle_analyse_result != nResult)
	{
		m_face_angle_analyse_result = nResult;
		LoadGif(GetgifFileName(nResult).c_str());
	}
		
}

void  CAigoCamera::LoadGif(const wchar_t* gifFileName)
{

	CSAutoLock lock(&m_gifLocker);
	if (m_pGif)
	{
		delete m_pGif;
		m_pGif = NULL;
	}

	if (!gifFileName || gifFileName[0] != L'\0')
		return;

	m_pGif = new Image(gifFileName);
	m_nFrameIndex = 0;
	if (m_pGif->GetLastStatus() == Ok)
	{
		m_nFrameCount = m_pGif->GetFrameCount(&FrameDimensionTime);
		m_guidDimension = FrameDimensionTime;

		// 读取帧延迟（PropertyTagFrameDelay = 0x5100）
		UINT size = m_pGif->GetPropertyItemSize(PropertyTagFrameDelay);
		if (size > 0)
		{
			PropertyItem* pItem = (PropertyItem*)malloc(size);
			if (pItem && m_pGif->GetPropertyItem(PropertyTagFrameDelay, size, pItem) == Ok)
			{
				BYTE* delays = (BYTE*)pItem->value;
				m_nFrameDelay = ((UINT*)delays)[0] * 10; // 以10ms为单位
			}
			free(pItem);
		}

		//SetTimer(GetSafeHwnd(), 1, m_nFrameDelay, NULL);
	}
}

int  CAigoCamera::GetGifFrameDelay()
{
	return m_nFrameDelay;
}

void CAigoCamera::DrawGif(Gdiplus::Graphics* g)
{
	CSAutoLock lock(&m_gifLocker);


	if (!m_pGif) return;

	
	m_pGif->SelectActiveFrame(&m_guidDimension, m_nFrameIndex);

	int imgWidth = m_pGif->GetWidth()/2;
	int imgHeight = m_pGif->GetHeight()/2;

	// 计算左上角位置，使图片在中下方显示
	Rect rc;
	g->GetVisibleClipBounds(&rc);
	int x = (rc.Width - imgWidth)/2;
	int y = (rc.Height - imgHeight);
	g->DrawImage(m_pGif, x, y, imgWidth, imgHeight);
	m_nFrameIndex = (m_nFrameIndex + 1) % m_nFrameCount;
	// 半透明覆盖（在 GIF 上绘制透明文字或图形）
	//SolidBrush semiTransparentBrush(Color(128, 255, 0, 0)); // A=128 红色半透明
	//graphics.FillRectangle(&semiTransparentBrush, Rect(10, 10, 200, 50));

	//FontFamily fontFamily(L"Segoe UI");
	//Gdiplus::Font font(&fontFamily, 18, FontStyleRegular, UnitPixel);
	//SolidBrush textBrush(Color(200, 255, 255, 255)); // 白色文字，略微透明
	//graphics.DrawString(L"这是透明叠加", -1, &font, PointF(20, 15), &textBrush);
}

void CAigoCamera::Draw3VLine(HDC hDC, int nWidth, int nHeight)
{
	int nXStep = nWidth / 4;
	int nYStep = nHeight / 6;

	// 创建绿色虚线笔
	HPEN hPen = CreatePen(PS_DASH, 1, RGB(0, 255, 0));
	HGDIOBJ hOldPen = SelectObject(hDC, hPen);

	SetBkMode(hDC, TRANSPARENT);

	// 画三条竖线
	MoveToEx(hDC, nXStep, nYStep, nullptr);
	LineTo(hDC, nXStep, nYStep * 5);

	MoveToEx(hDC, nXStep * 2, nYStep, nullptr);
	LineTo(hDC, nXStep * 2, nYStep * 5);

	MoveToEx(hDC, nXStep * 3, nYStep, nullptr);
	LineTo(hDC, nXStep * 3, nYStep * 5);

	// 清理
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}


void CAigoCamera::Draw3VLine(Gdiplus::Graphics *g, int nWidth, int nHeight)
{
	int nYStep = nHeight / 6;
	int nXStep = nWidth / 4;
	int nX = nXStep;
	int nY = nYStep;

	Gdiplus::Pen pen(Color::Green, Gdiplus::REAL(1.0));
	pen.SetDashStyle(DashStyleDash);

	for (int i = 0; i < 3; i++)
	{
		g->DrawLine(&pen, nX + nXStep*i, nYStep, nX + nXStep*i, nYStep*5);
	}

}


void CAigoCamera::PreviewShowVideo(HWND hwnd, HDC hDC, RECT &rc)
{
	if (!m_lpBuf1)
		return;

	std::lock_guard<std::mutex> lock(m_frameMutex);

	// 位图文件头
	char m1 = 'B';
	char m2 = 'M';
	short res1 = 0;
	short res2 = 0;
	long pixoff = 54;

	LPBYTE pBmpBuf = new BYTE[0x200000];
	if (!pBmpBuf)
	{
		//theApp.GetMainWnd()->MessageBox(_T("Create pBmpBuf error"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"Create pBmpBuf error.");

		return;
	}
	memset(pBmpBuf, 0x0, sizeof(BYTE) * 0x200000);
	// 宽度
	DWORD widthDW = WIDTHBYTES(m_usVideo_w * 24);

	// 文件长度
	long bmfsize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + widthDW * m_usVideo_h;

	// 文件指针
	long byteswritten = 0;

	// 位图信息头
	BITMAPINFOHEADER header;
	header.biSize = 40; 		   		    // 头大小
	header.biWidth = m_usVideo_w;
	header.biHeight = m_usVideo_h;
	header.biPlanes = 1;
	header.biBitCount = 24;					// 24位
	header.biCompression = BI_RGB;			// 非压缩
	header.biSizeImage = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biClrUsed = 0;
	header.biClrImportant = 0;

	///////////////////////////////////////////////////////////////////////////

	long row = 0;
	long rowidx;
	long row_size;

	row_size = header.biWidth * 3;

	// 保存位图阵列
	char dummy = 0;
	DWORD count;
	for (row = 0; row < header.biHeight; row++)
		//for (row = header.biHeight-1; row >=0; row--)
	{
		rowidx = (long unsigned)row * row_size;

		// 写一行
		memcpy(pBmpBuf + byteswritten, (void*)(m_lpBuf1 + rowidx), row_size);
		byteswritten += row_size;

		for (count = row_size; count < widthDW; count++)
		{
			dummy = 0;
			memcpy(pBmpBuf + byteswritten, &dummy, 1);
			byteswritten++;
		}
	}

	int i = 0;
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);    // 头大小
	bmi.bmiHeader.biWidth = m_usVideo_w;
	bmi.bmiHeader.biHeight = m_usVideo_h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;					// 24位
	bmi.bmiHeader.biCompression = BI_RGB;			// 非压缩
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	for (i = 0; i < bmi.bmiColors[i].rgbBlue; i++)
	{
		bmi.bmiColors[i].rgbGreen = i;
		bmi.bmiColors[i].rgbRed = i;
		bmi.bmiColors[i].rgbReserved = 0;
	}

	//int nWidth = rc.Width();
	//int nHeight = rc.Height();
	//DrawWell(pDC, rc.Width(), rc.Height());
	//Draw3VLine(pDC, rc.Width(), rc.Height());
	//CGdiplusInit gdiplusInit;
	Gdiplus::Bitmap img(&bmi, pBmpBuf);
	//if (0 == GetPreviewRotateValue())
	//	img.RotateFlip(RotateFlipType::Rotate90FlipY);
	//else
	//	img.RotateFlip(RotateFlipType::Rotate270FlipY);

	if (!m_bAnalyseStop)
	{// do save face angle analyse file.
		SaveFaceAngleAnalyseFile(&img);
	}

	Gdiplus::RectF rcImg(0, 0, (Gdiplus::REAL)img.GetWidth(), (Gdiplus::REAL)img.GetHeight());

	//img.
	//Gdiplus::Bitmap b(rc.Width(), rc.Height());
	//Gdiplus::Graphics *gMEM = Graphics::FromImage(&b);
	Gdiplus::Graphics* gMEM = Graphics::FromImage(&img);
	Gdiplus::RectF rcTo(0, 0, 
						static_cast<Gdiplus::REAL>(rc.right - rc.left),
						static_cast<Gdiplus::REAL>(rc.bottom - rc.top));
	//gMEM->DrawImage(&img, rcTo, 0, 0, rcFrom.Width, rcFrom.Height, UnitPixel);

	if (rc.bottom - rc.top > 500 && !m_strEdgFileName.empty() && PathFileExists(m_strEdgFileName.c_str()) == TRUE)
	{
		Gdiplus::Bitmap img2(m_strEdgFileName.c_str());
		Gdiplus::RectF rcFrom(0, 0, Gdiplus::REAL(img2.GetWidth()), Gdiplus::REAL(img2.GetHeight()));
		//ColorMatrix ClrMatrix = {
		//	1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//	0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		//	0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//	0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
		//	0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		//};
		ImageAttributes imgAttributes;
		//imgAttributes.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault,ColorAdjustTypeBitmap);
		//imgAttributes.SetColorKey(Color(0, 0, 0, 0), Color(0, 0, 0, 0), ColorAdjustTypeDefault);
		imgAttributes.SetColorKey(Color(255, 255, 255, 255), Color(255, 255, 255, 255), ColorAdjustTypeDefault);
		gMEM->DrawImage(&img2, rcImg, 0, 0, rcFrom.Width, rcFrom.Height, UnitPixel, &imgAttributes);
	}
	else
	{
		Draw3VLine(gMEM, (int)rcImg.Width, (int)rcImg.Height);
	}
	//gMEM->DrawImage(&img, rcTo, 0, 0, rcFrom.Width, rcFrom.Height, UnitPixel, &imgAttributes);
	//gMEM->Save();
	//Draw3VLine(gMEM, (int)rcImg.Width, (int)rcImg.Height);
	DrawGif(gMEM);
	Gdiplus::Graphics* g = Graphics::FromHDC(hDC);
	g->DrawImage(&img, rcTo, 0, 0, (Gdiplus::REAL)img.GetWidth(), (Gdiplus::REAL)img.GetHeight(), UnitPixel);

	delete gMEM;
	delete[] pBmpBuf;

}

void CAigoCamera::DrawWell(HDC hDC, int nWidth, int nHeight)
{
	int nXStep = nWidth / 3;
	int nYStep = nHeight / 3;

	// 创建红色实线画笔
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HGDIOBJ hOldPen = SelectObject(hDC, hPen);

	// 画横线
	MoveToEx(hDC, 0, nYStep, nullptr);
	LineTo(hDC, nWidth, nYStep);

	MoveToEx(hDC, 0, nYStep * 2, nullptr);
	LineTo(hDC, nWidth, nYStep * 2);

	// 画竖线
	MoveToEx(hDC, nXStep, 0, nullptr);
	LineTo(hDC, nXStep, nHeight);

	MoveToEx(hDC, nXStep * 2, 0, nullptr);
	LineTo(hDC, nXStep * 2, nHeight);

	// 恢复旧画笔并释放资源
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}

//long CAigoCamera::Capture2(BSTR* pbstrFileName)
//{
//	LOG(L"start Capture2"));
//	// TODO: Add your dispatch handler code here
//
//											// Allocate buffer and initialize buffer value for WriteFile.
//
//	while (!OneVideoFrameFinish) { Sleep(1); }
//	m_bPause = true;
//
//	LOG(L"Capture2 after OneVideoFrameFinish"));
//	//while (KZIsLensMoving()) { Sleep(500); }
//	while (KZIsDoingAF()) { Sleep(500); }
//
//	LOG(L"Capture2 after KZIsLensMoving"));
//	CString strPath(*pbstrFileName);
//	if (strPath.IsEmpty())
//	{
//		if (!KZCapture())
//			//if(!KZIsUseCaptureAF(1))
//		{
//			//theApp.GetMainWnd()->MessageBox(_T("KZCapture failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
//			LOG(L"KZCapture failed"));
//			m_bPause = false;
//			return -1;
//		}
//		//Sleep(10000);
//		LOG(L"Capture2 after KZCapture"));
//
//	}
//	else
//	{
//		DWORD	fileReturned;
//		DWORD	FileSaveReturned;
//		LPVOID	lpBuffer;
//		DWORD	dwBufferLength = 0x800000;		// 4M-byte.
//		lpBuffer = new BYTE[dwBufferLength];
//		if (lpBuffer == NULL)
//			return -1;
//		memset(lpBuffer, 0x00, dwBufferLength);
//		fileReturned = KZUpload(lpBuffer);
//		LOG(L"Capture2 after KZUpload");
//
//		if (fileReturned == UNDEF)
//		{
//			//theApp.GetMainWnd()->MessageBox(_T("Read Image Failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
//			LOG(L"Read Image Failed");
//		}
//
//		//strPath = "V10.jpg";
//		//////////////////////////////////+++
//		HANDLE hFile = CreateFile((LPCTSTR)strPath,
//			GENERIC_READ | GENERIC_WRITE,  // read-write access
//			0,                             // NO share
//			NULL,                          // NO security specification
//			CREATE_ALWAYS,                 //
//			FILE_ATTRIBUTE_NORMAL,         //
//			NULL);                        // No template
//
//		if (hFile != INVALID_HANDLE_VALUE)
//		{
//			WriteFile(hFile, lpBuffer, fileReturned, &FileSaveReturned, NULL);
//			CloseHandle(hFile);
//		}
//		delete[]lpBuffer;
//		LOG(L"Capture2 after WriteFile");
//	}
//	m_bPause = false;
//	LOG(L"end Capture2");
//
//	return 0;
//}

bool CAigoCamera::doCapture(const wchar_t* pstrFileName)
{
	LOG(L"start Capture");
	// TODO: Add your dispatch handler code here
	DWORD	fileReturned;
	DWORD	FileSaveReturned;
	LPVOID	lpBuffer;
	DWORD	dwBufferLength = 0x800000;		// 4M-byte.

	// Allocate buffer and initialize buffer value for WriteFile.
	lpBuffer = new BYTE[dwBufferLength];
	if (lpBuffer == NULL)
		return false;
	memset(lpBuffer, 0x00, dwBufferLength);

	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;

	LOG(L"Capture after OneVideoFrameFinish");
	while (KZIsLensMoving()) { Sleep(500); }

	LOG(L"Capture after KZIsLensMoving");

	if (!KZCapture())
		//if(!KZIsUseCaptureAF(1))
	{
		//theApp.GetMainWnd()->MessageBox(_T("KZCapture failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZCapture failed");
		delete[]lpBuffer;
		lpBuffer = NULL;
		m_bPause = false;
		return false;
	}
	LOG(L"Capture after KZCapture");
	//Sleep(1000);

	fileReturned = KZUpload(lpBuffer);
	LOG(L"Capture after KZUpload");

	if (fileReturned == UNDEF)
	{
		//theApp.GetMainWnd()->MessageBox(_T("Read Image Failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"Read Image Failed");
		return false;
	}

	//strPath = "V10.jpg";
	//////////////////////////////////+++
	HANDLE hFile = CreateFile(pstrFileName,
		GENERIC_READ | GENERIC_WRITE,  // read-write access
		0,                             // NO share
		NULL,                          // NO security specification
		CREATE_ALWAYS,                 //
		FILE_ATTRIBUTE_NORMAL,         //
		NULL);                        // No template

	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuffer, fileReturned, &FileSaveReturned, NULL);
		CloseHandle(hFile);
	}
	delete[]lpBuffer;
	LOG(L"Capture after WriteFile");

	m_bPause = false;
	LOG(L"end Capture");

	return true;

}


long CAigoCamera::Capture(std::function<void()> callback)
{
	int nILLTypes[] = {ILL_RGB_TYPE,
  				  ILL_365UV_TYPE,
				  ILL_405UV_TYPE,
				  ILL_PL_TYPE,
				  ILL_NPL_TYPE };
	const int nCount = sizeof(nILLTypes) / sizeof(nILLTypes[0]);
	this->Autofocusing(0);

	for (int i = 0; i < nCount; ++i) {
		int type = nILLTypes[i];
		if (!Util::Instance().IsAutoCreate(type))
		{
			BeforeCapture(type);
			doCapture(GetCaptureFileName(type).c_str());
			AfterCapture(type);
		}
	}

	callback();
	
	return 0;

}


void CAigoCamera::Gray(BitmapData *data)
{
	PARGBQuad p = (PARGBQuad)data->Scan0;
	INT offset = (data->Stride - data->Width * sizeof(ARGBQuad)) / sizeof(ARGBQuad);

	for (UINT y = 0; y < data->Height; y++, p += offset)
	{
		for (UINT x = 0; x < data->Width; x++, p++)
			p->Blue = p->Green = p->Red =
			(UINT)(p->Blue * 29 + p->Green * 150 + p->Red * 77 + 128) >> 8;
	}
}

void ConvertToGrayscale(Bitmap *source)
{
	int Height = source->GetHeight();
	int Width = source->GetWidth();
	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			Color c;
			source->GetPixel(x, y, &c);
			BYTE average = (BYTE)((c.GetRed()* 77 + c.GetGreen() * 150 + c.GetBlue() * 29)>>8);
			source->SetPixel(x, y, Color(average, average, average));
		}
	}
}
void CAigoCamera::CreateEdgePicture(const wchar_t* pszFrom, const wchar_t* pszTo, const RECT& rc, int nThreshold)
{
	//CGdiplusInit gdiplusInit;

	Gdiplus::Bitmap imageold(pszFrom);
	INT iWidthOld = imageold.GetWidth();
	INT iHeightOld = imageold.GetHeight();

	int nWitdh = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	Bitmap b(nWitdh, nHeight);
	Bitmap* b2;

	Gdiplus::RectF rcFrom(0, 0, Gdiplus::REAL(iWidthOld), Gdiplus::REAL(iHeightOld));
	Gdiplus::RectF rcTo(0, 0, (Gdiplus::REAL)nWitdh, (Gdiplus::REAL)nHeight);

	Gdiplus::Graphics* g = Graphics::FromImage(&b);
	g->DrawImage(&imageold, rcTo, 0, 0, rcFrom.Width, rcFrom.Height, UnitPixel);

	b2 = b.Clone(rcTo, PixelFormat24bppRGB);

	ConvertToGrayscale(b2);

	BitmapData bmData;
	BitmapData bmData2;
	Gdiplus::Rect rect(0, 0, nWitdh, nHeight);
	b.LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat24bppRGB, &bmData);
	b2->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat24bppRGB, &bmData2);

	int stride = bmData.Stride;

	unsigned char* p = (unsigned char*)bmData.Scan0;
	unsigned char* p2 = (unsigned char*)bmData2.Scan0;


	int nOffset = stride - rect.Width * 3;
	//int nWidth = iWidth * 3;

	int nPixel = 0, nPixelMax = 0;

	p += stride;
	p2 += stride;

	//int nThreshold = 20;
	do_analyse(rect.Height, rect.Width, p, p2, stride, nThreshold);

	b.UnlockBits(&bmData);
	b2->UnlockBits(&bmData2);

	std::wstring strFileName = pszTo;
	CLSID jpgClsid;
	CGdiplusInit::GetEncoderClsid(L"image/bmp", &jpgClsid);
	b.Save(strFileName.c_str(), &jpgClsid, NULL);

	// add for auto create mirror edg file from L2R or R2L
	std::wstring strFileName2 = strFileName;

	size_t pos = strFileName2.find(L"_L__edg.bmp");
	if (pos != std::wstring::npos)
	{
		// 替换 'L' 为 'R'
		strFileName2.replace(pos + 1, 1, L"R");
		MirrorBmpImage(strFileName.c_str(), strFileName2.c_str());
	}
	else
	{
		pos = strFileName2.find(L"_R__edg.bmp");
		if (pos != std::wstring::npos)
		{
			// 替换 'R' 为 'L'
			strFileName2.replace(pos + 1, 1, L"L");
			MirrorBmpImage(strFileName.c_str(), strFileName2.c_str());
		}
	}


}

// 镜像函数
bool CAigoCamera::MirrorBmpImage(const wchar_t* inputPath, const wchar_t* outputPath)
{
	Image image(inputPath);
	if (image.GetLastStatus() != Ok) return false;

	image.RotateFlip(RotateNoneFlipX);

	CLSID bmpClsid;
	if (CGdiplusInit::GetEncoderClsid(L"image/bmp", &bmpClsid) <0) return false;

	return image.Save(outputPath, &bmpClsid, nullptr) == Ok;
}

void CAigoCamera::SetCaptureType(LONG nType)
{
}

int CAigoCamera::GetCameraSeries()
{
	return Aigo;
}

std::string CAigoCamera::GetFrame()
{
	if (this->IsPausePreview())
		return "";

	std::string frameCopy;
	{
		std::lock_guard<std::mutex> lock(m_frameMutex);
		frameCopy = m_oneframeBase64;
	}
	return frameCopy;
}


void CAigoCamera::ReqOneFrame(std::function<void()> callback)
{
	if (!IsPausePreview()) {
		OneVideoFrameFinish = false;

		BOOL bRet =	KZPreviewOneVideoFrameRGB(m_lpBuf1);
		OneVideoFrameFinish = true;

		std::string base64 = BmpBufferToBase64Jpeg(m_lpBuf1,this->m_usVideo_w,m_usVideo_h);

		// 🔒 写入共享帧（上锁）
		{
			std::lock_guard<std::mutex> lock(m_frameMutex);
			m_oneframeBase64 = std::move(base64);
		}
		//SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
		callback();
	}
}

std::string  CAigoCamera::BmpBufferToBase64Jpeg(LPBYTE pBmpBuf, int width, int height)
{
	// 创建 GDI+ Bitmap（注意stride）
	int stride = width * 3; // 24-bit BGR
	Bitmap bmp(width, height, stride, PixelFormat24bppRGB, pBmpBuf);

	// 保存到内存流（JPEG 格式）
	CLSID clsidEncoder;
	UINT num = 0, size = 0;
	GetImageEncodersSize(&num, &size);
	std::vector<BYTE> buffer;
	if (size == 0) return "";

	std::unique_ptr<ImageCodecInfo[]> pImageCodecInfo((ImageCodecInfo*)(malloc(size)));
	GetImageEncoders(num, size, pImageCodecInfo.get());
	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, L"image/jpeg") == 0)
		{
			clsidEncoder = pImageCodecInfo[i].Clsid;
			break;
		}
	}

	CComPtr<IStream> pStream;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	bmp.Save(pStream, &clsidEncoder, NULL);

	// 读取IStream内容
	STATSTG statstg;
	pStream->Stat(&statstg, STATFLAG_NONAME);
	ULONG sizeBytes = (ULONG)statstg.cbSize.QuadPart;
	buffer.resize(sizeBytes);

	LARGE_INTEGER liZero = {};
	pStream->Seek(liZero, STREAM_SEEK_SET, NULL);
	ULONG bytesRead = 0;
	pStream->Read(buffer.data(), sizeBytes, &bytesRead);

	// Base64 编码
	//DWORD base64Len = 0;
	//CryptBinaryToStringA(buffer.data(), bytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64Len);
	//std::string base64Str(base64Len, '\0');
	//CryptBinaryToStringA(buffer.data(), bytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64Str.data(), &base64Len);

	DWORD base64Len = 0;
	CryptBinaryToStringA(buffer.data(), bytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64Len);
	std::string base64Str(base64Len - 1, '\0'); // 减掉结尾 \0
	CryptBinaryToStringA(buffer.data(), bytesRead, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64Str.data(), &base64Len);


	// 拼出完整 data URL
	return base64Str;
}

bool CAigoCamera::GetSupportWBs(std::vector<int>& values)
{
	return false;
}

bool CAigoCamera::GetSupportISOs(std::vector<int>& values)
{
	return false;
}

bool CAigoCamera::GetSupportApertures(std::vector<int>& values)
{
	return false;
}

bool CAigoCamera::GetSupportExposures(std::vector<int>& values)
{
	return false;
}

void CAigoCamera::ReqOneFrame2()
{
}

std::vector<uint8_t> CAigoCamera::GetFrame2()
{
	return std::vector<uint8_t>();
}

void CAigoCamera::do_analyse(int iHeight, int iWidth, unsigned char * p, unsigned char * p2, int stride, int nThreshold)
{
	const int WHITE = 255;
	const int BLACK = 0;

	int nOffset = stride - iWidth * 3;
	int nWidth = iWidth * 3;

	int nPixel = 0, nPixelMax = 0;

	for (int y = 1; y<iHeight - 1; ++y)
	{
		p += 3;
		p2 += 3;

		for (int x = 3; x < nWidth - 3; x += 3)
		{


			nPixelMax = abs((p2 - stride + 3)[0] - (p2 + stride - 3)[0]);
			nPixel = abs((p2 + stride + 3)[0] - (p2 - stride - 3)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			nPixel = abs((p2 - stride)[0] - (p2 + stride)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			nPixel = abs((p2 + 3)[0] - (p2 - 3)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			if (nPixelMax <= nThreshold)
			{
				p[0] = WHITE;// (byte)nPixelMax;
				p[1] = WHITE;
				p[2] = WHITE;

			}
			else
			{
				p[0] = 0;// (byte)nPixelMax;
				p[1] = WHITE;
				p[2] = 0;

			}
			p += 3;
			p2 += 3;
		}
		p += 3 + nOffset;
		p2 += 3 + nOffset;
	}
}


void CAigoCamera::SetISO(int iso)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZISOSet(iso))
		//theApp.GetMainWnd()->MessageBox(_T("KZSetPreviewISO failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZSetPreviewISO failed");
	m_bPause = false;

}

void CAigoCamera::SetWB(int wb)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;

	if (!KZAWBMode((USHORT)wb, 0))
		//theApp.GetMainWnd()->MessageBox(_T("KZAWBMode failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZAWBMode failed");

	if (!KZSharpMode(0))
		//theApp.GetMainWnd()->MessageBox(_T("KZSharpMode failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZSharpMode failed");

	m_bPause = false;
}

void CAigoCamera::SetExposure(int exposure)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZASM_TVSET((USHORT)exposure))
		//theApp.GetMainWnd()->MessageBox(_T("KZSetCaptureExposure failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZSetCaptureExposure failed");
	m_bPause = false;
}

void CAigoCamera::SetZoom(int zoom)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;

	if (zoom >= 0 && zoom <= 10)
	{
		LOG(std::format(L"SetZoom KZZoom = {}", zoom));


		while (KZIsLensMoving()) {
			Sleep(500); 
		}
		if (!KZZoom((USHORT)zoom))
			//theApp.GetMainWnd()->MessageBox(_T("KZZoom failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"KZZoom failed");
	}
	//m_ZoomOld = zoom;
	m_bPause = false;
}

void CAigoCamera::Autofocusing(int usauto)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;

	//if (!KZIsUseCaptureAF(usauto))
	if (usauto)
	{
		LOG(L"KZAf On");
		if (!KZAF())
			//theApp.GetMainWnd()->MessageBox(_T("KZAf failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"KZAf failed");
	}
	else
	{
		LOG(L"KZAf Off");
	}

	m_bPause = false;


}

void CAigoCamera::SetFocusType(int focusType)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	
	if (!KZMeterMode(1))
		LOG(L"KZAFModeSet failed");

	if (!KZAFMode(focusType)) // 0: Normal , 1: 近焦对焦
		//theApp.GetMainWnd()->MessageBox(_T("KZAFModeSet failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZAFModeSet failed");

	m_bPause = false;

}

void CAigoCamera::SetFlash(bool bflash)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	
	USHORT nDuration = 500;
	if (bflash)
	{
		if (!KZFlashModeSet(5))
			//theApp.GetMainWnd()->MessageBox(_T("KZFlashModeSet failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"KZFlashModeSet failed");
		if (!KZFirePower_OutsideFlash(nDuration))
			//theApp.GetMainWnd()->MessageBox(_T("KZFirePower_OutsideFlash failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"KZFirePower_OutsideFlash failed");
		LOG(L"start charge");
		KZFlashCharge(1);
		if (KZIsFlashCharging())		//判断闪光灯是否充电完成
		{
			int flash_charge_time = 0;
			do
			{
				flash_charge_time++;
				if (flash_charge_time>100)
				{
					//theApp.GetMainWnd()->MessageBox(_T("KZIsFlashCharging error!"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
					LOG(L"KZIsFlashCharging error");
					return;	//闪光灯充电错误！退出
				}
				Sleep(200);
			} while (KZIsFlashCharging());
		}
		KZFlashCharge(0);
		LOG(L"end charge");
		KZFireOutsideFlash();
	}
	else
		if (!KZFlashModeSet(4))
			//theApp.GetMainWnd()->MessageBox(_T("KZFlashModeSet failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"KZFlashModeSet failed");
	m_bPause = false;
}

void CAigoCamera::SetImgSize(int imgSize)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZImageSizeSet(imgSize))
		//theApp.GetMainWnd()->MessageBox(_T("KZAFModeSet failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZAFModeSet failed");
	m_bPause = false;
}

void CAigoCamera::SetImgQuality(int imgQuality)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZSetQuality(imgQuality))
		//theApp.GetMainWnd()->MessageBox(_T("KZSetQuality failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZSetQuality failed");
	m_bPause = false;
}

void CAigoCamera::SetAperture(int aperture)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZASM_AVSET(aperture))
		//theApp.GetMainWnd()->MessageBox(_T("KZASM_AVSET failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZASM_AVSET failed");
	m_bPause = false;
}

void CAigoCamera::SetFocusPos(int nPos)
{
	while (!OneVideoFrameFinish) { Sleep(1); }
	m_bPause = true;
	if (!KZFocusPos((USHORT)nPos))
		//theApp.GetMainWnd()->MessageBox(_T("KZFocusPos failed"), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
		LOG(L"KZFocusPos failed");
	m_bPause = false;
}

void CAigoCamera::SetMode(int mode)
{
	if (!m_bInit) return;

	while (!OneVideoFrameFinish) { Sleep(1); }

	m_bPause = true;

	switch (mode)
	{
	case PMode:
		if (!KZEnterPMode())
		{
			//theApp.GetMainWnd()->MessageBox(_T("P Mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"P Mode setting error.");
		}
		break;
	case MMode:
		if (!KZEnterMMode())
		{
			//theApp.GetMainWnd()->MessageBox(_T("M Mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"M Mode setting error.");
		}
		break;
	case AMode:
		if (!KZEnterAMode())
		{
			//theApp.GetMainWnd()->MessageBox(_T("A Mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"A Mode setting error.");
		}
		break;
	case SMode:
		if (!KZEnterSMode())
		{
			//theApp.GetMainWnd()->MessageBox(_T("S Mode setting error."), _T("vantonT1000drv"), MB_ICONWARNING | MB_OK);
			LOG(L"S Mode setting error.");
		}
		break;
	}

	Sleep(2000);	//wait mode change;

	m_bPause = false;

}

void CAigoCamera::SetEdgeFileName(const wchar_t* pbstrFileName)
{
	m_strEdgFileName = pbstrFileName ? pbstrFileName : L"";
}

