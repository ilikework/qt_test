#pragma once
#include "CameraBase.h"
#include "EDSDK.h"
#include "EDSDKTypes.h"
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

struct CaptureContext {
    EdsCameraRef camera = nullptr;
    std::mutex mtx;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cv;
    bool running = true;
    HWND hwnd = nullptr;
};


class CanonEDSCamera : public ICameraBase
{
public:
    CanonEDSCamera();
    virtual ~CanonEDSCamera();


    // 通过 ICameraBase 继承
    long Init(const wchar_t* pszDllName) override;
    void unInit(bool bPowerOff) override;
    bool StartPreview(void) override;
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
    void PreviewShowVideo(HWND hwnd, HDC hDC, RECT& rc) override;
    void SetCaptureType(LONG nType) override;



protected:
	EdsBaseRef    m_hCamera;
    bool          m_bLegacy;
    EdsPropertyDesc  m_WBs;
    EdsPropertyDesc  m_ISOs;
    EdsPropertyDesc  m_AVs;
    EdsPropertyDesc  m_TVs;

    bool          m_bPreview;

    EdsError SetLocalStorage();
    EdsError ExtendShutdown();
    void GetFocusInfo();
	EdsError SetupCallback();
	EdsInt32 GetNearestValue(EdsPropertyID propID, EdsInt32 target) const;
	void cleanup(EdsEvfImageRef evfImage, EdsStreamRef stream);
	//EdsError DownloadPicture(EdsBaseRef inRef);
	long doCapture();
    void EndCatpure(const std::vector<std::string>& create_files);
	bool SaveCapturedItem(EdsDirectoryItemRef dirItem, const wchar_t* pstrFileName);

	static EdsError EDSCALLBACK  handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef, EdsVoid* inContext);
	static EdsError EDSCALLBACK  handlePropertyEvent(EdsUInt32 inEvent, EdsUInt32 inPropertyID, EdsUInt32 inParam, EdsVoid* inContext);
    static EdsError EDSCALLBACK  handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid* inContext);
    CaptureContext m_ctx;
    CGdiplusInit m_gdiplusInit;

    void startWorker();       // 启动工作线程
    void stopWorker();        // 停止工作线程
    void enqueueTask(std::function<void(void)> task);

    std::thread workerThread;
    bool stopFlag = false;
    std::queue<std::function<void(void)>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    void cameraWorker();

    //void CreatePreviewThread();
    //static unsigned _stdcall PreviewReadThreadBase(LPVOID lpPara);
    //void DoGetOneFrame();
    std::string m_oneframeBase64;// 当前可显示的帧
    std::mutex m_frameMutex;        // 互斥锁

private:
    CapturedCallback m_callback = nullptr;
    uint32_t m_id = 0;

    int GetCameraSeries() override;

    std::string GetFrame() override;
    void ReqOneFrame(std::function<void()> callback) override;

    // 通过 ICameraBase 继承
    bool GetSupportWBs(std::vector<int>& values) override;
    bool GetSupportISOs(std::vector<int>& values) override;
    bool GetSupportApertures(std::vector<int>& values) override;
    bool GetSupportExposures(std::vector<int>& values) override;

    // 通过 ICameraBase 继承
    void ReqOneFrame2() override;
    std::vector<uint8_t> GetFrame2() override;
    //std::mutex doneMutex_;
    std::condition_variable doneCv_;
    bool frameReady_ = false;
    std::vector<uint8_t> oneframe_;

    // 通过 ICameraBase 继承
    bool IsInited() override;


    // === 新增：同步用 ===
    std::mutex m_capMtx;
    std::condition_variable m_capCv;

    // “当前这张是否处理完成”
    bool m_oneShotDone = false;

    // 防止旧通知唤醒：每张递增一个序号
    uint64_t m_shotSeq = 0;          // Capture() 侧期望的序号
    uint64_t m_doneSeq = 0;
    // 通过 ICameraBase 继承
    void PushGetEvent() override;
    // 回调处理完毕的序号
};
