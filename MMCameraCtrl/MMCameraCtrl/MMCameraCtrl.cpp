// MMCameraCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

// VideoServer.cpp
// 32bit VC++ 相机 Server Demo
// - 监听 127.0.0.1:52345
// - 支持 JSON 命令 + 二进制帧数据
// - startPreview 后周期推送 dummy 帧
// - getPreviewFrame / capture 时立即返回一帧

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#include "protocol_common.h"

// ---------- 工具函数 ----------

// 可靠 recv：循环直到收满 len 字节或失败
bool recvAll(SOCKET s, char* buf, int len)
{
    int received = 0;
    while (received < len) {
        int ret = recv(s, buf + received, len - received, 0);
        if (ret <= 0) {
            return false; // 连接关闭或出错
        }
        received += ret;
    }
    return true;
}

// 可靠 send
bool sendAll(SOCKET s, const char* buf, int len)
{
    int sent = 0;
    while (sent < len) {
        int ret = send(s, buf + sent, len - sent, 0);
        if (ret <= 0) {
            return false;
        }
        sent += ret;
    }
    return true;
}

// 简单大小写不敏感查找
bool containsCmd(const std::string& json, const std::string& cmd)
{
    std::string s = json;
    std::string c = cmd;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);
    return s.find(c) != std::string::npos;
}

// ---------- 协议发送封装 ----------

// 发送 CommandResponse 的 JSON
bool sendCommandResponse(SOCKET s, uint32_t requestId, const std::string& cmd,
    int result, const std::string& message)
{
    // 简单拼 JSON，真实项目可以用 JSON 库
    std::string json = "{"
        "\"cmd\":\"" + cmd + "\","
        "\"result\":" + std::to_string(result) + ","
        "\"message\":\"" + message + "\""
        "}";

    MessageHeader hdr;
    hdr.msgType = htons((uint16_t)MSG_COMMAND_RESPONSE);
    hdr.version = htons((uint16_t)1);
    hdr.requestId = htonl(requestId);

    uint32_t bodyLen = (uint32_t)(sizeof(hdr) + json.size());
    uint32_t netBodyLen = htonl(bodyLen);

    std::vector<char> buf;
    buf.resize(4 + bodyLen);

    // 写入前导长度
    memcpy(buf.data(), &netBodyLen, 4);

    // 写入头
    memcpy(buf.data() + 4, &hdr, sizeof(hdr));

    // 写入 JSON
    memcpy(buf.data() + 4 + sizeof(hdr), json.data(), json.size());

    return sendAll(s, buf.data(), (int)buf.size());
}

// 发送一帧 dummy 图像
bool sendFrame(SOCKET s,
    uint32_t requestId,
    FrameType frameType,
    uint64_t seq,
    uint32_t width,
    uint32_t height,
    PixelFormat fmt)
{
    // 这里用固定大小的 dummy buffer，假装是 JPEG 数据
    const uint32_t dummySize = 50 * 1024; // 50KB
    std::vector<uint8_t> dummyData(dummySize, 0);

    FrameDataHeader fHdrHost = {};
    fHdrHost.frameType = (uint8_t)frameType;
    fHdrHost.reserved1 = 0;
    fHdrHost.reserved2 = 0;
    fHdrHost.seq = seq;
    fHdrHost.timestampMs = (uint64_t)GetTickCount64();
    fHdrHost.width = width;
    fHdrHost.height = height;
    fHdrHost.pixelFormat = (uint32_t)fmt;
    fHdrHost.dataLength = dummySize;

    // 转网络字节序
    FrameDataHeader fHdrNet = fHdrHost;
    fHdrNet.seq = htonll(fHdrNet.seq);
    fHdrNet.timestampMs = htonll(fHdrNet.timestampMs);
    fHdrNet.width = htonl(fHdrNet.width);
    fHdrNet.height = htonl(fHdrNet.height);
    fHdrNet.pixelFormat = htonl(fHdrNet.pixelFormat);
    fHdrNet.dataLength = htonl(fHdrNet.dataLength);

    MessageHeader msgHdr;
    msgHdr.msgType = htons((uint16_t)MSG_FRAME_DATA);
    msgHdr.version = htons((uint16_t)1);
    msgHdr.requestId = htonl(requestId);

    uint32_t bodyLen = (uint32_t)(sizeof(msgHdr) + sizeof(fHdrNet) + dummySize);
    uint32_t netBodyLen = htonl(bodyLen);

    std::vector<char> buf;
    buf.resize(4 + bodyLen);

    char* p = buf.data();
    memcpy(p, &netBodyLen, 4); p += 4;
    memcpy(p, &msgHdr, sizeof(msgHdr)); p += sizeof(msgHdr);
    memcpy(p, &fHdrNet, sizeof(fHdrNet)); p += sizeof(fHdrNet);
    memcpy(p, dummyData.data(), dummySize);

    return sendAll(s, buf.data(), (int)buf.size());
}

// ---------- Server 状态 ----------

struct ServerState
{
    bool     running = true;
    bool     previewOn = false;
    uint32_t previewWidth = 1280;
    uint32_t previewHeight = 720;
    PixelFormat previewFmt = PIX_FMT_JPEG;
    double   previewFps = 30.0;

    uint64_t frameSeq = 1;
};

// ---------- 处理收到的单个完整 message ----------

bool handleMessage(SOCKET clientSock,
    const MessageHeader& hdrHost,
    const std::vector<char>& body,
    ServerState& state)
{
    // body = [MessageHeader][payload...]
    if (body.size() < sizeof(MessageHeader)) {
        std::cerr << "Invalid body size: " << body.size() << std::endl;
        return false;
    }

    const char* payload = body.data() + sizeof(MessageHeader);
    int payloadLen = (int)(body.size() - sizeof(MessageHeader));

    switch (hdrHost.msgType) {
    case MSG_COMMAND_REQUEST:
    {
        std::string json(payload, payloadLen);
        std::cout << "[REQ] requestId=" << hdrHost.requestId
            << " json=" << json << std::endl;

        // 简单通过字符串判断 cmd，真实项目建议用 JSON 解析库
        if (containsCmd(json, "\"cmd\":\"opencamera\"")) {
            // 这里你可以做真正的打开相机处理
            sendCommandResponse(clientSock, hdrHost.requestId,
                "openCamera", 0, "OK");
        }
        else if (containsCmd(json, "\"cmd\":\"closecamera\"")) {
            sendCommandResponse(clientSock, hdrHost.requestId,
                "closeCamera", 0, "OK");
        }
        else if (containsCmd(json, "\"cmd\":\"startpreview\"")) {
            state.previewOn = true;
            state.frameSeq = 1;
            sendCommandResponse(clientSock, hdrHost.requestId,
                "startPreview", 0, "OK");
        }
        else if (containsCmd(json, "\"cmd\":\"stoppreview\"")) {
            state.previewOn = false;
            sendCommandResponse(clientSock, hdrHost.requestId,
                "stopPreview", 0, "OK");
        }
        else if (containsCmd(json, "\"cmd\":\"getpreviewframe\"")) {
            // 单帧：回一帧，frameType=FRAME_TYPE_SINGLE
            sendCommandResponse(clientSock, hdrHost.requestId,
                "getPreviewFrame", 0, "OK");
            sendFrame(clientSock, hdrHost.requestId,
                FRAME_TYPE_SINGLE,
                state.frameSeq++,
                state.previewWidth, state.previewHeight,
                state.previewFmt);
        }
        else if (containsCmd(json, "\"cmd\":\"capture\"")) {
            // 拍照：回一帧，frameType=FRAME_TYPE_CAPTURE
            sendCommandResponse(clientSock, hdrHost.requestId,
                "capture", 0, "OK");
            sendFrame(clientSock, hdrHost.requestId,
                FRAME_TYPE_CAPTURE,
                state.frameSeq++,
                state.previewWidth, state.previewHeight,
                state.previewFmt);
        }
        else {
            // 未知命令
            sendCommandResponse(clientSock, hdrHost.requestId,
                "unknown", -1, "Unknown cmd");
        }

        break;
    }

    case MSG_HEARTBEAT:
        std::cout << "[HEARTBEAT] requestId=" << hdrHost.requestId << std::endl;
        break;

    default:
        std::cout << "[UNKNOWN MSG] type=" << hdrHost.msgType
            << " len=" << body.size() << std::endl;
        break;
    }

    return true;
}

// ---------- 从 socket 读一个完整 message（阻塞直到读完或失败） ----------

bool recvOneMessage(SOCKET clientSock, std::vector<char>& outBody, MessageHeader& hdrHost)
{
    // 先读 4 字节 bodyLen
    uint32_t netBodyLen = 0;
    if (!recvAll(clientSock, (char*)&netBodyLen, sizeof(netBodyLen))) {
        return false;
    }
    uint32_t bodyLen = ntohl(netBodyLen);
    if (bodyLen < sizeof(MessageHeader)) {
        std::cerr << "Invalid bodyLen: " << bodyLen << std::endl;
        return false;
    }

    outBody.resize(bodyLen);
    if (!recvAll(clientSock, outBody.data(), bodyLen)) {
        return false;
    }

    // 解析头
    memcpy(&hdrHost, outBody.data(), sizeof(MessageHeader));
    hdrHost.msgType = ntohs(hdrHost.msgType);
    hdrHost.version = ntohs(hdrHost.version);
    hdrHost.requestId = ntohl(hdrHost.requestId);

    return true;
}

// ---------- main ----------

int main()
{
    // 1. 初始化 winsock
    WSADATA wsaData;
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r != 0) {
        std::cerr << "WSAStartup failed: " << r << std::endl;
        return 1;
    }

    // 2. 创建监听 socket
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    int opt = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(52345);              // 固定端口
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 只监听本机，不会触发防火墙

    if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on 127.0.0.1:52345, waiting for client..." << std::endl;

    // 3. accept 一个客户端
    sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &clientLen);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // 4. 主循环：select + 收命令 + （按FPS推 dummy 预览帧）
    ServerState state;
    ULONGLONG lastFrameTick = GetTickCount64();
    double frameIntervalMs = 1000.0 / state.previewFps;

    while (state.running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSock, &readfds);

        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 30 * 1000; // 30ms 超时，便于周期推帧

        int ret = select(0, &readfds, NULL, NULL, &tv);
        if (ret == SOCKET_ERROR) {
            std::cerr << "select error: " << WSAGetLastError() << std::endl;
            break;
        }

        if (ret > 0 && FD_ISSET(clientSock, &readfds)) {
            // 有数据可读：读一个完整 message
            std::vector<char> body;
            MessageHeader hdrHost;
            if (!recvOneMessage(clientSock, body, hdrHost)) {
                std::cout << "Client disconnected or error." << std::endl;
                break;
            }

            if (!handleMessage(clientSock, hdrHost, body, state)) {
                std::cout << "handleMessage failed." << std::endl;
                break;
            }
        }

        // 没数据可读时，也会每 30ms 回来一次，这里判断是否要推预览帧
        if (state.previewOn) {
            ULONGLONG now = GetTickCount64();
            if (now - lastFrameTick >= (ULONGLONG)frameIntervalMs) {
                // 推一帧 preview
                if (!sendFrame(clientSock,
                    0, // 预览流：requestId=0，也可以用 startPreview 的 requestId
                    FRAME_TYPE_PREVIEW,
                    state.frameSeq++,
                    state.previewWidth,
                    state.previewHeight,
                    state.previewFmt)) {
                    std::cout << "send preview frame failed (disconnected?)." << std::endl;
                    break;
                }
                lastFrameTick = now;
            }
        }
    }

    closesocket(clientSock);
    closesocket(listenSock);
    WSACleanup();

    std::cout << "Server exit." << std::endl;
    return 0;
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
