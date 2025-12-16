#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include "CameraBase.h"

// ================== 协议定义（示例） ==================
// 如果你已有协议头文件，请删除这一段并 include 你自己的协议定义
enum MsgType : uint16_t {
    MSG_COMMAND_REQUEST  = 1,
    MSG_COMMAND_RESPONSE = 2,
    MSG_FRAME_DATA       = 3,
    MSG_HEARTBEAT        = 4,
};

enum FrameType : uint8_t {
    FRAME_TYPE_PREVIEW = 1,
    FRAME_TYPE_SINGLE  = 2,
    FRAME_TYPE_CAPTURE = 3,
};

enum MMPixelFormat : uint32_t {
    PIX_FMT_JPEG = 1,
};

#pragma pack(push, 1)
struct MessageHeader {
    uint16_t msgType;
    uint16_t version;
    uint32_t requestId;
};

struct FrameDataHeader {
    uint8_t  frameType;
    uint8_t  reserved1;
    uint16_t reserved2;
    uint64_t seq;
    uint64_t timestampMs;
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormat;
    uint32_t dataLength;
};
#pragma pack(pop)

// ================== SocketServer ==================
class SocketServer {
public:
    struct Config {
        std::string bindIp = "127.0.0.1";
        uint16_t    port   = 52345;

        double      previewFps    = 30.0;
        uint32_t    previewWidth  = 1280;
        uint32_t    previewHeight = 720;
        MMPixelFormat previewFmt    = PIX_FMT_JPEG;
    };

    explicit SocketServer(const Config& cfg);
    ~SocketServer();

    // 初始化 winsock + 创建监听 socket + bind + listen
    bool start();

    // accept 一个客户端并进入主循环（阻塞）
    void run();

    // 关闭 socket（可多次调用）
    void stop();

private:
    // winsock
    bool initWinsock();
    void cleanupWinsock();

    // listen / accept
    bool createListenSocket();
    bool bindAndListen();
    bool acceptOneClient();

    // io helpers
    static bool recvAll(SOCKET s, char* buf, int len);
    static bool sendAll(SOCKET s, const char* buf, int len);
    
    // protocol send/recv
    bool sendCommandResponse(uint32_t requestId,const std::string& cmd, const std::string& result);

    bool sendFrame(uint32_t requestId,
                   FrameType frameType,
                   uint64_t seq,
                   uint32_t width,
                   uint32_t height,
                   PixelFormat fmt);

    bool recvOneMessage(std::vector<char>& outBody, MessageHeader& hdrHost);

    // message handler
    bool handleMessage(const MessageHeader& hdrHost, const std::vector<char>& body);

private:
    Config cfg_;

    bool winsockInited_ = false;
    bool running_ = false;

    SOCKET listenSock_ = INVALID_SOCKET;
    SOCKET clientSock_ = INVALID_SOCKET;

    bool previewOn_ = false;
    uint64_t frameSeq_ = 1;
    bool exitRequested_ = false;

    std::unique_ptr<ICameraBase> pCamera_ = std::make_unique<NullCamera>();

};
