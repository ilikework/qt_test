#pragma once
#include "EDSDK.h"
#include "EDSDKTypes.h"
#include <atomic>
#include <stdexcept>

class CanonEDSUILock {
public:
    explicit CanonEDSUILock(EdsCameraRef cam)
        : camera(cam), ownsLock(false)
    {
        int expected = 0;
        // 如果当前无人持有锁
        if (lockCount.compare_exchange_strong(expected, 1)) {
            // 我是第一个持有者 -> 真正发锁命令
            auto err = EdsSendStatusCommand(camera, kEdsCameraStatusCommand_UILock, 0);
            if (err != EDS_ERR_OK) {
                lockCount.store(0); // 回滚
                throw std::runtime_error("UILock failed");
            }
            ownsLock = true;
        }
        else {
            // 已经有人锁了，我只是共享持有
            lockCount.fetch_add(1);
        }
    }

    ~CanonEDSUILock() {
        release();
    }

    // 禁止拷贝，支持移动
    CanonEDSUILock(const CanonEDSUILock&) = delete;
    CanonEDSUILock& operator=(const CanonEDSUILock&) = delete;

    CanonEDSUILock(CanonEDSUILock&& other) noexcept {
        camera = other.camera;
        ownsLock = other.ownsLock;
        other.ownsLock = false;
    }

    CanonEDSUILock& operator=(CanonEDSUILock&& other) noexcept {
        if (this != &other) {
            release();
            camera = other.camera;
            ownsLock = other.ownsLock;
            other.ownsLock = false;
        }
        return *this;
    }

private:
    void release() {
        if (camera == nullptr) return;

        // 减计数
        int count = lockCount.fetch_sub(1);

        if (count == 1 && ownsLock) {
            // 我是最后一个，并且是最初的加锁者
            EdsSendStatusCommand(camera, kEdsCameraStatusCommand_UIUnLock, 0);
        }
        ownsLock = false;
    }

    EdsCameraRef camera;
    bool ownsLock;

    // 全局计数器
    static std::atomic<int> lockCount;
};

// 静态成员初始化
std::atomic<int> CanonEDSUILock::lockCount{ 0 };
