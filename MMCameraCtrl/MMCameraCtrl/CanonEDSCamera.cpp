#include "pch.h"
#include "CanonEDSCamera.h"
#include "CanonEDSUILock.h"
#include <atlbase.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

// Live View 质量
#define kEdsEvfQuality_Low   1
#define kEdsEvfQuality_High  2
#define kEdsEvfQuality_Movie 3

// Live View 属性 ID
#define kEdsPropID_Evf_Quality 0x00000211

CanonEDSCamera::CanonEDSCamera()
	:m_bPreview(false), m_hCamera(nullptr), m_bLegacy(false)
{
	memset(&m_WBs, 0, sizeof(EdsPropertyDesc));
	memset(&m_ISOs, 0, sizeof(EdsPropertyDesc));
	memset(&m_AVs, 0, sizeof(EdsPropertyDesc));
	memset(&m_TVs, 0, sizeof(EdsPropertyDesc));
}

CanonEDSCamera::~CanonEDSCamera()
{
	unInit(true);
}

EdsError CanonEDSCamera::SetLocalStorage()
{
	bool locked = false;
	EdsUInt32 saveTo = kEdsSaveTo_Host;
	EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_SaveTo, 0, sizeof(saveTo), &saveTo);
	//UI lock
	//if (err == EDS_ERR_OK)
	//{
	//	err = EdsSendStatusCommand(m_hCamera, kEdsCameraStatusCommand_UILock, 0);
	//}

	//if (err == EDS_ERR_OK)
	//{
	//	locked = true;
	//}

	if (err == EDS_ERR_OK)
	{
		EdsCapacity capacity = { 0x7FFFFFFF, 0x1000, 1 };
		err = EdsSetCapacity(m_hCamera, capacity);
	}

	//It releases it when locked
	//if (locked)
	//{
	//	EdsSendStatusCommand(m_hCamera, kEdsCameraStatusCommand_UIUnLock, 0);
	//}

	return err;
}

EdsError CanonEDSCamera::ExtendShutdown()
{
	bool locked = false;
	EdsError err = EDS_ERR_OK;
	//UI lock
	//EdsError err = EdsSendStatusCommand(m_hCamera, kEdsCameraStatusCommand_UILock, 0);

	//if (err == EDS_ERR_OK)
	//{
	//	locked = true;
	//}

	if (err == EDS_ERR_OK)
	{
		err = EdsSendCommand(m_hCamera, kEdsCameraCommand_ExtendShutDownTimer, 0);
	}

	//It releases it when locked
	//if (locked)
	//{
	//	EdsSendStatusCommand(m_hCamera, kEdsCameraStatusCommand_UIUnLock, 0);
	//}
	return err;
}


EdsError CanonEDSCamera::SetupCallback()
{
	EdsError	 err = EDS_ERR_OK;
	//Set Property Event Handler
	if (err == EDS_ERR_OK)
	{
		err = EdsSetPropertyEventHandler(m_hCamera, kEdsPropertyEvent_All, CanonEDSCamera::handlePropertyEvent, (EdsVoid*)this);
	}

	//Set Object Event Handler
	if (err == EDS_ERR_OK)
	{
		err = EdsSetObjectEventHandler(m_hCamera, kEdsObjectEvent_All, CanonEDSCamera::handleObjectEvent, (EdsVoid*)this);
	}

	//Set State Event Handler
	if (err == EDS_ERR_OK)
	{
		err = EdsSetCameraStateEventHandler(m_hCamera, kEdsStateEvent_All, CanonEDSCamera::handleStateEvent, (EdsVoid*)this);
	}
	return err;
}

EdsError EDSCALLBACK  CanonEDSCamera::handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef, EdsVoid * inContext)
{
	switch (inEvent)
	{
	case kEdsObjectEvent_DirItemRequestTransfer:
	{
		EdsRetain(inRef);

		((CanonEDSCamera*)inContext)->m_bPause = true;
		((CanonEDSCamera*)inContext)->enqueueTask([inRef, inContext]() {
			CanonEDSCamera* pParent = (CanonEDSCamera*)inContext;
			pParent->SaveCapturedItem(inRef, pParent->GetCaptureFileName(pParent->GetILLType()).c_str());
			EdsRelease(inRef);

			pParent->AfterCapture(pParent->GetILLType());
			if (pParent->GetILLType() != ILL_NPL_TYPE)
			{
				pParent->SetILLType(pParent->GetNextILLType());
				pParent->Capture(nullptr);
			}
			else
			{
				pParent->EndCatpure();
			}
			//pParent->Unlock(&pParent->cs_locker);

		//CaptureContext* ctx = static_cast<CaptureContext*>(inContext);
		//if (!ctx) return EDS_ERR_OK;

		//if (inRef) EdsRetain(inRef);

		//{
		//	std::lock_guard<std::mutex> lock(ctx->mtx);

		//	if (ctx->lastCapturedItem) {
		//		EdsRelease(ctx->lastCapturedItem);
		//		ctx->lastCapturedItem = nullptr;
		//	}

		//	ctx->lastCapturedItem = inRef;
		//	ctx->ready = true;
		//}
		//ctx->cv.notify_one();
			((CanonEDSCamera*)inContext)->m_bPause = false;
		});
		return EDS_ERR_OK;
	}
	default:
		//Object without the necessity is released
		if (inRef != NULL)
		{
			EdsRelease(inRef);
		}
		break;
	}

	return EDS_ERR_OK;
}

EdsError EDSCALLBACK  CanonEDSCamera::handlePropertyEvent(EdsUInt32 inEvent, EdsUInt32 inPropertyID, EdsUInt32 inParam, EdsVoid* inContext)
{
	return EDS_ERR_OK;
}

EdsError EDSCALLBACK  CanonEDSCamera::handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid* inContext)
{
	return EDS_ERR_OK;
}


long CanonEDSCamera::Init(const wchar_t* pszDllName)
{
	LOG(L"CanonEDSCamera::Init start");
		
	EdsError	 err = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	EdsCameraRef camera = NULL;
	EdsUInt32	 count = 0;
	bool		 isSDKLoaded = false;

	// Initialization of SDK
	err = EdsInitializeSDK();

	if (err == EDS_ERR_OK)
	{
		isSDKLoaded = true;
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsInitializeSDK return={}", err));


	//Acquisition of camera list
	if (err == EDS_ERR_OK)
	{
		err = EdsGetCameraList(&cameraList);
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetCameraList return={}", err));

	//Acquisition of number of Cameras
	if (err == EDS_ERR_OK)
	{
		err = EdsGetChildCount(cameraList, &count);
		if (count == 0)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetChildCount return={}", err));

	//Acquisition of camera at the head of the list
	if (err == EDS_ERR_OK)
	{
		err = EdsGetChildAtIndex(cameraList, 0, &m_hCamera);
		//g_ctx.camera = m_hCamera;

	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetChildAtIndex return={}", err));

	//Acquisition of camera information
	EdsDeviceInfo deviceInfo;
	if (err == EDS_ERR_OK)
	{
		err = EdsGetDeviceInfo(m_hCamera, &deviceInfo);
		if (err == EDS_ERR_OK && m_hCamera == NULL)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetDeviceInfo return={}", err));

	//Release camera list
	if (cameraList != NULL)
	{
		EdsRelease(cameraList);
	}

	//Create Camera model
	bool bOpenSession = false;
	if (err == EDS_ERR_OK)
	{
		m_bLegacy = (deviceInfo.deviceSubType == 0) ? true : false;
		err = EdsOpenSession(m_hCamera);
		bOpenSession = true;
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsOpenSession return={}", err));

	//Preservation ahead is set to PC
	if (err == EDS_ERR_OK)
	{
		err = SetLocalStorage();
	}
	LOG(std::format(L"CanonEDSCamera::Init SetLocalStorage return={}", err));

	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyDesc(m_hCamera, kEdsPropID_WhiteBalance, &m_WBs);
	}
	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyDesc(m_hCamera,kEdsPropID_ISOSpeed,&m_ISOs);
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetPropertyDesc.kEdsPropID_ISOSpeed return={}", err));

	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyDesc(m_hCamera, kEdsPropID_Av, &m_AVs);
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetPropertyDesc.kEdsPropID_Av return={}", err));

	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyDesc(m_hCamera, kEdsPropID_Tv, &m_TVs);
	}
	LOG(std::format(L"CanonEDSCamera::Init EdsGetPropertyDesc.kEdsPropID_Tv return={}", err));

	if (err == EDS_ERR_OK)
	{
		err = ExtendShutdown();
	}
	LOG(std::format(L"CanonEDSCamera::Init ExtendShutdown return={}", err));

	if (err != EDS_ERR_OK)
	{
		if(bOpenSession)
			EdsCloseSession(m_hCamera);

		if (m_hCamera)
		{
			EdsRelease(m_hCamera);
			m_hCamera = NULL;
		}
			
		if(isSDKLoaded)
			EdsTerminateSDK();
		return -1;
	}

	return 0;
}

void CanonEDSCamera::unInit(bool bPowerOff)
{
	StopPreview();

	if (m_hCamera)
	{
		EdsCloseSession(m_hCamera);
		EdsRelease(m_hCamera);
		m_hCamera = NULL;
	}

	EdsTerminateSDK();
}

void CanonEDSCamera::startWorker() {
	stopFlag = false;
	workerThread = std::thread(&CanonEDSCamera::cameraWorker, this);
	// 注意：绑定成员函数要用 &ClassName::func, this
}

void CanonEDSCamera::stopWorker() {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		stopFlag = true;
	}
	cv.notify_all();

	if (workerThread.joinable()) {
		workerThread.join();
	}
}

void CanonEDSCamera::enqueueTask(std::function<void(void)> task) {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		taskQueue.push(std::move(task));
	}
	cv.notify_one();
}

void CanonEDSCamera::cameraWorker() {
	while (true) {
		std::function<void(void)> task;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			cv.wait(lock, [this] { return stopFlag || !taskQueue.empty(); });

			if (stopFlag && taskQueue.empty())
				break;  // 退出线程

			task = std::move(taskQueue.front());
			taskQueue.pop();
		}

		if (task) task();
	}
}

int CanonEDSCamera::GetCameraSeries()
{
	return CanonEOS;
}

std::string CanonEDSCamera::GetFrame()
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

void CanonEDSCamera::ReqOneFrame(std::function<void()> callback)
{
	if (this->IsPausePreview())
		return;
	enqueueTask([this, callback]() {

		if (this->IsPausePreview())
			return;
		EdsStreamRef stream = nullptr;
		EdsEvfImageRef evfImage = nullptr;
		bool result = false;

		// 创建内存流
		EdsError err = EdsCreateMemoryStream(2 * 1024 * 1024, &stream);
		if (err != EDS_ERR_OK) return;

		// 创建 EVF 图像对象
		err = EdsCreateEvfImageRef(stream, &evfImage);
		if (err != EDS_ERR_OK) {
			EdsRelease(stream);
			return;
		}

		// 下载 EVF 图像
		err = EdsDownloadEvfImage(m_hCamera, evfImage);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}

		//EdsUInt32 format = 0;
		//EdsGetPropertyData(evfImage, kEdsPropID_Evf_Mode, 0, sizeof(format), &format);
		//LOG(std::format(L"EVF format = {}", format));

		// 获取数据指针
		EdsUInt64 length = 0;
		void* pData = nullptr;
		err = EdsGetPointer(stream, (EdsVoid**)&pData);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}
		err = EdsGetLength(stream, &length);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}


		HGLOBAL hMem = ::GlobalAlloc(GHND, (SIZE_T)length);
		LPVOID pBuff = ::GlobalLock(hMem);

		memcpy(pBuff, pData, (SIZE_T)length);

		::GlobalUnlock(hMem);

		IStream* pStream = nullptr;
		if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) {
			::GlobalFree(hMem);
			return;
		}

		Gdiplus::Image image(pStream);
		pStream->Release();  // GDI+ 内部会引用，不用担心
	
		if (image.GetLastStatus() != Ok)
			return ;

		// 3. 创建缩放后的 Bitmap
		Bitmap resized(320, 240, PixelFormat24bppRGB);
		{
			Graphics g(&resized);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			g.DrawImage(&image, 0, 0, 320, 240);
		}

		// 4. 编码为 JPEG 到 IStream
		IStream* outStream = nullptr;
		if (CreateStreamOnHGlobal(NULL, TRUE, &outStream) != S_OK)
			return ;

		CLSID clsidEncoder;
		// 获取 JPEG 编码器 CLSID
		UINT numEncoders = 0, sizeEncoders = 0;
		GetImageEncodersSize(&numEncoders, &sizeEncoders);
		if (sizeEncoders == 0) {
			outStream->Release();
			return ;
		}
		std::vector<BYTE> buf(sizeEncoders);
		ImageCodecInfo* pEncoders = reinterpret_cast<ImageCodecInfo*>(buf.data());
		GetImageEncoders(numEncoders, sizeEncoders, pEncoders);
		for (UINT i = 0; i < numEncoders; i++) {
			if (wcscmp(pEncoders[i].MimeType, L"image/jpeg") == 0) {
				clsidEncoder = pEncoders[i].Clsid;
				break;
			}
		}

		EncoderParameters params;
		params.Count = 1;
		ULONG quality = 80;
		params.Parameter[0].Guid = EncoderQuality;
		params.Parameter[0].Type = EncoderParameterValueTypeLong;
		params.Parameter[0].NumberOfValues = 1;
		params.Parameter[0].Value = &quality;

		if (resized.Save(outStream, &clsidEncoder, &params) != Ok) {
			outStream->Release();
			return ;
		}

		// 5. 从 IStream 获取内存
		HGLOBAL hGlobal = nullptr;
		if (GetHGlobalFromStream(outStream, &hGlobal) != S_OK) {
			outStream->Release();
			return ;
		}
		SIZE_T len = GlobalSize(hGlobal);
		BYTE* pData2 = (BYTE*)GlobalLock(hGlobal);

		// 6. Base64 编码
		DWORD base64Len = 0;
		if (!CryptBinaryToStringA((unsigned char*)pData2, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64Len))
		{
			GlobalUnlock(hGlobal);
			outStream->Release();
			return ;
		}
		std::string base64;
		base64.resize(base64Len);
		CryptBinaryToStringA((unsigned char*)pData2, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64.data(), &base64Len);

		GlobalUnlock(hGlobal);
		outStream->Release();

		// 去掉末尾 '\0'
		if (!base64.empty() && base64.back() == '\0')
			base64.pop_back();


		//std::string base64 = Util::Instance().EncodeBase64Frame((unsigned char*)pData, (size_t)length);

		// 🔒 写入共享帧（上锁）
		{
			std::lock_guard<std::mutex> lock(m_frameMutex);
			m_oneframeBase64 = std::move(base64);
		}

		if (evfImage) EdsRelease(evfImage);
		if (stream) EdsRelease(stream);
		callback();
		});
}

bool CanonEDSCamera::GetSupportWBs(std::vector<int>& values)
{
	values.resize(m_WBs.numElements);
	for (int i = 0; i < this->m_WBs.numElements; ++i) 
	{
		values[i] = static_cast<int>(m_WBs.propDesc[i]);
	}
	return true;
}

bool CanonEDSCamera::GetSupportISOs(std::vector<int>& values)
{
	values.resize(m_ISOs.numElements);
	for (int i = 0; i < this->m_ISOs.numElements; ++i)
	{
		values[i] = static_cast<int>(m_ISOs.propDesc[i]);
	}
	return true;
}

bool CanonEDSCamera::GetSupportApertures(std::vector<int>& values)
{
	values.resize(m_AVs.numElements);
	for (int i = 0; i < this->m_AVs.numElements; ++i)
	{
		values[i] = static_cast<int>(m_AVs.propDesc[i]);
	}
	return true;
}

bool CanonEDSCamera::GetSupportExposures(std::vector<int>& values)
{
	values.resize(m_TVs.numElements);
	for (int i = 0; i < this->m_TVs.numElements; ++i)
	{
		values[i] = static_cast<int>(m_TVs.propDesc[i]);
	}
	return true;
}

void CanonEDSCamera::ReqOneFrame2(std::function<void(void*)> callback, void* param)
{
}

std::vector<uint8_t> CanonEDSCamera::GetFrame2()
{
	return std::vector<uint8_t>();
}

bool CanonEDSCamera::StartPreview(void)
{
	if (m_bPreview)
		return true;

	EdsError err = EDS_ERR_OK;
	if (err == EDS_ERR_OK)
	{
		err = SetupCallback();
	}

	if (err == EDS_ERR_OK)
	{
		EdsUInt32 evfMode = 1; // start preview.
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_Evf_Mode, 0, sizeof(evfMode), &evfMode);
	}

	//Acquisition of the property size
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;
	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertySize(m_hCamera,kEdsPropID_Evf_OutputDevice,0,&dataType,&dataSize);
	}
	EdsUInt32 data;
	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyData(m_hCamera, kEdsPropID_Evf_OutputDevice, 0, dataSize, &data);
	}

	if (err == EDS_ERR_OK)
	{
		// Set the PC as the current output device.
		data |= kEdsEvfOutputDevice_PC;

		// Set to the camera.
		err = EdsSetPropertyData(m_hCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(data), &data);
	}

	if (err == EDS_ERR_OK)
	{
		// Set to the camera.
		data = Evf_AFMode_Live;
		err = EdsSetPropertyData(m_hCamera, kEdsPropID_Evf_AFMode, 0, sizeof(data), &data);
	}

	if (err != EDS_ERR_OK)
	{
		LOG(L"StartPreview error");
		return false;
	}

	startWorker();

	//if (Util::Instance().GetCaptureSetting(CanonEOS, m_captureSetting))
	{
		//CanonEDSUILock uilock(m_hCamera);

		this->SetWB(m_captureSetting.get_rgb().wb);
		this->SetISO(m_captureSetting.get_rgb().iso);
		this->SetAperture(m_captureSetting.get_rgb().aperture);
		this->SetExposure(m_captureSetting.get_rgb().exposure);
		//this->SetImgQuality(m_captureSetting.getImgQuality());
		//this->SetImgSize(m_captureSetting.getImgSize());
	}

	m_bPreview = true;
	this->ResumePreview();

	//CreatePreviewThread();
	return true;
}

//void CanonEDSCamera::CreatePreviewThread()
//{
//	TRACE(L"CanonEDSCamera::CreatePreviewThread\n");
//	m_bPause = false;
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

//unsigned _stdcall CanonEDSCamera::PreviewReadThreadBase(LPVOID lpPara)
//{
//	BOOL bRet;
//	CanonEDSCamera* pSelf = (CanonEDSCamera*)lpPara;
//
//	while (pSelf->m_bPreview)
//	{
//		if (!pSelf->m_bPause) {
//			pSelf->DoGetOneFrame();
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(60)); // 控制帧率
//	}
//	return 0;
//}

//void CanonEDSCamera::DoGetOneFrame()
//{
//	if (m_bPause)
//		return;
//	enqueueTask([this]() {
//		
//		EdsStreamRef stream = nullptr;
//		EdsEvfImageRef evfImage = nullptr;
//		bool result = false;
//
//		// 创建内存流
//		EdsError err = EdsCreateMemoryStream(2 * 1024 * 1024, &stream);
//		if (err != EDS_ERR_OK) return;
//
//		// 创建 EVF 图像对象
//		err = EdsCreateEvfImageRef(stream, &evfImage);
//		if (err != EDS_ERR_OK) {
//			EdsRelease(stream);
//			return;
//		}
//
//		// 下载 EVF 图像
//		err = EdsDownloadEvfImage(m_hCamera, evfImage);
//		if (err != EDS_ERR_OK) {
//			if (evfImage) EdsRelease(evfImage);
//			if (stream) EdsRelease(stream);
//			return;
//		}
//
//		//EdsUInt32 format = 0;
//		//EdsGetPropertyData(evfImage, kEdsPropID_Evf_Mode, 0, sizeof(format), &format);
//		//LOG(std::format(L"EVF format = {}", format));
//
//		// 获取数据指针
//		EdsUInt64 length = 0;
//		void* pData = nullptr;
//		err = EdsGetPointer(stream, (EdsVoid**)&pData);
//		if (err != EDS_ERR_OK) {
//			if (evfImage) EdsRelease(evfImage);
//			if (stream) EdsRelease(stream);
//			return;
//		}
//		err = EdsGetLength(stream, &length);
//		if (err != EDS_ERR_OK) {
//			if (evfImage) EdsRelease(evfImage);
//			if (stream) EdsRelease(stream);
//			return;
//		}
//
//		std::string base64 = Util::Instance().EncodeBase64Frame((unsigned char*)pData, (size_t)length);
//
//		// 🔒 写入共享帧（上锁）
//		{
//			std::lock_guard<std::mutex> lock(m_frameMutex);
//			m_oneframeBase64 = std::move(base64);
//			m_newFrame = true;
//		}
//
//		
//
//		//// 把内存数据交给 GDI+ 解码为 Bitmap
//		////HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)length);
//		////if (!hMem) {
//		////	cleanup(evfImage, stream);
//		////	return ;
//		////}
//		////void* pMem = GlobalLock(hMem);
//		////memcpy(pMem, pData, (size_t)length);
//		////GlobalUnlock(hMem);
//
//		//HGLOBAL hMem = ::GlobalAlloc(GHND, (SIZE_T)length);
//		//LPVOID pBuff = ::GlobalLock(hMem);
//
//		//memcpy(pBuff, pData, (SIZE_T)length);
//
//		//::GlobalUnlock(hMem);
//
//		//IStream* pStream = nullptr;
//		//if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) {
//		//	::GlobalFree(hMem);
//		//	OneVideoFrameFinish = true;
//		//	return;
//		//}
//
//		//Gdiplus::Image image(pStream);
//		//pStream->Release();  // GDI+ 内部会引用，不用担心
//		//
//		//if (image.GetLastStatus() == Ok) {
//		//	// 获取绘制区域
//		//
//		//	// 在设备 DC 上绘制
//		//	Gdiplus::Graphics graphics(hDC);
//		//	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
//		//	graphics.DrawImage(&image, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
//		//}
//
//		if (evfImage) EdsRelease(evfImage);
//		if (stream) EdsRelease(stream);
//		});
//}

void CanonEDSCamera::GetFocusInfo()
{
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;
	EdsError	err = EdsGetPropertySize(m_hCamera, kEdsPropID_FocusInfo, 0, &dataType, &dataSize);

	EdsUInt32 data;
	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertyData(m_hCamera, kEdsPropID_FocusInfo, 0, dataSize, &data);
	}


}

void CanonEDSCamera::StopPreview(void)
{
	if (!m_bPreview)
		return;

	this->PausePreview();
	stopWorker();

	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;
	EdsError	err = EdsGetPropertySize(m_hCamera, kEdsPropID_Evf_OutputDevice, 0, &dataType, &dataSize);
	if (err == EDS_ERR_OK)
	{
		// Set the PC as the current output device.
		EdsUInt32 data = kEdsEvfOutputDevice_TFT;

		// Set to the camera.
		err = EdsSetPropertyData(m_hCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(data), &data);
	}

	if (err == EDS_ERR_OK)
	{
		EdsUInt32 evfMode = 0; // stop preview.
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_Evf_Mode, 0, sizeof(evfMode), &evfMode);
	}

	if (err != EDS_ERR_OK)
	{
		LOG(L"StopPreview error");
		return ;
	}

	m_bPreview = false;
}

long CanonEDSCamera::Capture(std::function<void()> callback)
{
	if (m_type == ILL_NONE_TYPE)
		m_type = ILL_RGB_TYPE;
	if (!Util::Instance().IsAutoCreate(m_type))
	{
		BeforeCapture(m_type);
		doCapture();
	}
	else if (GetILLType() != ILL_NPL_TYPE)
	{
		// do next.
		SetILLType(GetNextILLType());
		Capture(callback);
	}

	if (callback != nullptr)
		m_callback = callback;

	return 0;
}

void CanonEDSCamera::EndCatpure()
{
	if (m_callback)
		m_callback();

	SetILLType(ILL_NONE_TYPE);
}

long CanonEDSCamera::doCapture()
{
	//{
	//	std::unique_lock<std::mutex> lock(m_ctx.mtx);
	//	m_ctx.ready = false;
	//	if (m_ctx.lastCapturedItem)
	//	{
	//		EdsRelease(m_ctx.lastCapturedItem);
	//		m_ctx.lastCapturedItem = nullptr;
	//	}
	//}
	//m_bPause = true;
	enqueueTask([this]() {

		EdsError err = EDS_ERR_OK;
		bool	 locked = false;

		//Taking a picture
		if (err == EDS_ERR_OK)
		{
			err = EdsSendCommand(m_hCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Halfway_NonAF);

		}
		if (err == EDS_ERR_OK)
		{
			err = EdsSendCommand(m_hCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely);
		}
		if (err == EDS_ERR_OK)
		{
			err = EdsSendCommand(m_hCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_OFF);
		}
		//this->m_bPause = false;
		if (err != EDS_ERR_OK)
		{
			LOG(L"Capture error");
			return -1;
		}
		return 0;

		// 4. 等待 DirItemRequestTransfer
		//EdsDirectoryItemRef capturedItem = nullptr;
		//{
		//	std::unique_lock<std::mutex> lock(m_ctx.mtx);
		//	m_ctx.cv.wait(lock, [this] { return this->m_ctx.ready; });
		//	capturedItem = m_ctx.lastCapturedItem;
		//}

		//if (capturedItem) {
		//	std::wstring str(pstrFileName);
		//	bool ret = SaveCapturedItem(capturedItem, pstrFileName);
		//	EdsRelease(capturedItem);
		//	if (!ret) {
		//		LOG(L"save error");
		//		return -1;
		//	}
		//}
	});

	return 0;
}

bool CanonEDSCamera::SaveCapturedItem(EdsDirectoryItemRef dirItem, const wchar_t* pstrFileName)
{
	if (!dirItem) return false;

	EdsDirectoryItemInfo dirInfo;
	if (EdsGetDirectoryItemInfo(dirItem, &dirInfo) != EDS_ERR_OK) return false;

	EdsStreamRef stream = nullptr;
	if (EdsCreateFileStream(Util::Instance().WStringToString(pstrFileName).c_str(),
		kEdsFileCreateDisposition_CreateAlways,
		kEdsAccess_ReadWrite,
		&stream) != EDS_ERR_OK) return false;

	if (EdsDownload(dirItem, dirInfo.size, stream) != EDS_ERR_OK) {
		EdsRelease(stream);
		return false;
	}
	EdsDownloadComplete(dirItem);
	EdsRelease(stream);
	return true;
}


EdsInt32 CanonEDSCamera::GetNearestValue(EdsPropertyID propID, EdsInt32 target) const 
{
	const EdsPropertyDesc* pDesc = nullptr;
	switch (propID) {
	case kEdsPropID_ISOSpeed: pDesc = &m_ISOs; break;
	case kEdsPropID_Av:       pDesc = &m_AVs;   break;
	case kEdsPropID_Tv:       pDesc = &m_TVs;   break;
	default:
		return -1;
	}

	if (!pDesc || pDesc->numElements == 0)
		return -1;

	// 假设 propDesc 排序后，找到第一个大于等于 target 的值
	for (EdsInt32 i = 0; i < pDesc->numElements; ++i) {
		if (pDesc->propDesc[i] >= target) {
			return pDesc->propDesc[i];
		}
	}

	return -1;
}

void CanonEDSCamera::SetISO(int iso)
{
	//CanonEDSUILock uilock(m_hCamera);

	//EdsInt32 data = GetNearestValue(kEdsPropID_ISOSpeed, iso);
	EdsInt32 data = iso;
	//this->m_bPause = true;
	enqueueTask([this, data]() {
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_ISOSpeed, 0, sizeof(data), (EdsVoid*)&data);
		if (err != EDS_ERR_OK)
		{
			//LOG(L"SetISO error");
			LOG(std::format(L"CanonEDSCamera::SetISO error iso = {} return={}", data,err));
		}
		//this->m_bPause = false;

	});
}

void CanonEDSCamera::SetWB(int wb)
{
	//CanonEDSUILock uilock(m_hCamera);
	//this->m_bPause = true;
	enqueueTask([this, wb]() {

		EdsInt32 data = wb;
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_WhiteBalance, 0, sizeof(data), (EdsVoid*)&data);
		if (err != EDS_ERR_OK)
		{
			LOG(std::format(L"CanonEDSCamera::SetAWBMode error awbMode = {} return={}", wb, err));
		}
		//this->m_bPause = false;
	});
}

void CanonEDSCamera::SetExposure(int exposure)
{
	//CanonEDSUILock uilock(m_hCamera);

	//EdsInt32 data = GetNearestValue(kEdsPropID_Tv, exposure);
	EdsInt32 data = exposure;
	//this->m_bPause = true;
	enqueueTask([this, data]() {
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_Tv, 0, sizeof(data), (EdsVoid*)&data);
		if (err != EDS_ERR_OK)
		{
			LOG(std::format(L"CanonEDSCamera::SetAWBMode error exposure = {} return={}", data, err));
		}
		//this->m_bPause = false;
	});
}

void CanonEDSCamera::SetZoom(int zoom)
{ 
	// Canon EDS Camera cannot SetZoom.
}

void CanonEDSCamera::Autofocusing(int usauto)
{
	//  Canon EDS Camera not use auto focus.
}

void CanonEDSCamera::SetFocusType(int focusType)
{
	//  Canon EDS Camera not use micro focus(近焦).
}

void CanonEDSCamera::SetFlash(bool bflash)
{
	//  Canon EDS Camera not use flash
}

void CanonEDSCamera::SetImgSize(int imgSize)
{
	//CanonEDSUILock uilock(m_hCamera);
	//this->m_bPause = true;
	enqueueTask([this,imgSize]() {

			EdsUInt32 lImageQuality = kEdsCompressQuality_Fine; // 固定给Fine质量，不做SetImgQuality

			//if (imgSize == kEdsImageSize_Middle1 || imgSize == kEdsImageSize_Middle2) 
			{
				lImageQuality = 0;
			}

			EdsUInt32 lImageSQ = (imgSize << 24) + (kEdsImageType_Jpeg << 20) + (lImageQuality << 16) + 0x0000FF0F;

			EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_ImageQuality, 0, sizeof(lImageSQ), &lImageSQ);
			if (err != EDS_ERR_OK) {
				LOG(L"SetImgSize error");
			}
			//this->m_bPause = false;
		});
}

void CanonEDSCamera::SetImgQuality(int imgQuality)
{
	// Canon EDS Camera not use imgQuality setting, and it is default set by SetImgSize as Fine.
}

void CanonEDSCamera::SetAperture(int aperture)
{
	//CanonEDSUILock uilock(m_hCamera);

	//EdsInt32 data = GetNearestValue(kEdsPropID_Av, aperture);
	EdsInt32 data = aperture;
	//this->m_bPause = true;
	enqueueTask([this, data]() {
		EdsError err = EdsSetPropertyData(m_hCamera, kEdsPropID_Av, 0, sizeof(data), (EdsVoid*)&data);
		if (err != EDS_ERR_OK)
		{
			LOG(std::format(L"CanonEDSCamera::SetAperture error aperture = {} return={}", data, err));
		}
		//this->m_bPause = false;
	});
}

void CanonEDSCamera::SetFocusPos(int nPos)
{
	// Canon EDS Camera not use SetFocusPos
}

void CanonEDSCamera::SetMode(int mode)
{
	//  Canon EDS Camera not use SetMode, for can only support Manual mode.
}

int CanonEDSCamera::GetMode()
{
	//  Canon EDS Camera not use SetMode, for can only support Manual mode.
	return 0;
}

bool CanonEDSCamera::IsPreviewStop()
{
	return m_bPreview ? false:true;
}

void CanonEDSCamera::cleanup(EdsEvfImageRef evfImage, EdsStreamRef stream) 
{
	if (evfImage) EdsRelease(evfImage);
	if (stream) EdsRelease(stream);
}

void CanonEDSCamera::PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc)
{
	if (this->IsPausePreview())
		return;
	Sleep(50);
	enqueueTask([this,hwnd, hDC, rc]() {
		EdsStreamRef stream = nullptr;
		EdsEvfImageRef evfImage = nullptr;
		bool result = false;

		// 创建内存流
		EdsError err = EdsCreateMemoryStream(2 * 1024 * 1024, &stream);
		if (err != EDS_ERR_OK) return;

		// 创建 EVF 图像对象
		err = EdsCreateEvfImageRef(stream, &evfImage);
		if (err != EDS_ERR_OK) {
			EdsRelease(stream);
			return;
		}

		// 下载 EVF 图像
		err = EdsDownloadEvfImage(m_hCamera, evfImage);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}

		//EdsUInt32 format = 0;
		//EdsGetPropertyData(evfImage, kEdsPropID_Evf_Mode, 0, sizeof(format), &format);
		//LOG(std::format(L"EVF format = {}", format));

		// 获取数据指针
		EdsUInt64 length = 0;
		void* pData = nullptr;
		err = EdsGetPointer(stream, (EdsVoid**)&pData);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}
		err = EdsGetLength(stream, &length);
		if (err != EDS_ERR_OK) {
			if (evfImage) EdsRelease(evfImage);
			if (stream) EdsRelease(stream);
			return;
		}

		// 把内存数据交给 GDI+ 解码为 Bitmap
		//HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)length);
		//if (!hMem) {
		//	cleanup(evfImage, stream);
		//	return ;
		//}
		//void* pMem = GlobalLock(hMem);
		//memcpy(pMem, pData, (size_t)length);
		//GlobalUnlock(hMem);

		HGLOBAL hMem = ::GlobalAlloc(GHND, (SIZE_T)length);
		LPVOID pBuff = ::GlobalLock(hMem);

		memcpy(pBuff, pData, (SIZE_T)length);

		::GlobalUnlock(hMem);

		IStream* pStream = nullptr;
		if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) {
			::GlobalFree(hMem);
			return;
		}

		Gdiplus::Image image(pStream);
		pStream->Release();  // GDI+ 内部会引用，不用担心

		if (image.GetLastStatus() == Ok) {
			// 获取绘制区域

			// 在设备 DC 上绘制
			Gdiplus::Graphics graphics(hDC);
			graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			graphics.DrawImage(&image, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
		}

		if (evfImage) EdsRelease(evfImage);
		if (stream) EdsRelease(stream);
	});
}	

void CanonEDSCamera::SetCaptureType(LONG nType)
{
}


