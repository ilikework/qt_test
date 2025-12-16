#include "SocketServer.h"

#include <ws2tcpip.h>
#include <iostream>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

// ================== 64-bit hton/ntoh ==================
static inline uint64_t bswap64(uint64_t x) {
    return ((x & 0x00000000000000FFULL) << 56) |
           ((x & 0x000000000000FF00ULL) << 40) |
           ((x & 0x0000000000FF0000ULL) << 24) |
           ((x & 0x00000000FF000000ULL) << 8)  |
           ((x & 0x000000FF00000000ULL) >> 8)  |
           ((x & 0x0000FF0000000000ULL) >> 24) |
           ((x & 0x00FF000000000000ULL) >> 40) |
           ((x & 0xFF00000000000000ULL) >> 56);
}
static inline uint64_t htonll(uint64_t x) { return bswap64(x); }
static inline uint64_t ntohll(uint64_t x) { return bswap64(x); }

// ================== ctor/dtor ==================
SocketServer::SocketServer(const Config& cfg)
    : cfg_(cfg) {}

SocketServer::~SocketServer() {
    stop();
    cleanupWinsock();
}

// ================== public ==================
bool SocketServer::start() {
    if (!initWinsock()) return false;
    if (!createListenSocket()) return false;
    if (!bindAndListen()) return false;
    running_ = true;
    return true;
}

void SocketServer::run() {
    if (!running_) {
        std::cerr << "Server not started.\n";
        return;
    }

    std::cout << "Server listening on " << cfg_.bindIp << ":" << cfg_.port
              << ", waiting for clients...\n";

    while (running_) {
        // 1) 等待一个 client
        if (!acceptOneClient()) {
            if (running_) { // 还在运行但 accept 失败
                std::cerr << "accept failed: " << WSAGetLastError() << "\n";
            }
            break;
        }

        std::cout << "Client connected.\n";

        // 2) reset per-client state
        previewOn_ = false;
        frameSeq_ = 1;
        exitRequested_ = false;

        ULONGLONG lastFrameTick = GetTickCount64();
        const double frameIntervalMs = 1000.0 / cfg_.previewFps;

        // 3) 服务当前 client，直到断开或 exit 命令
        while (running_ && clientSock_ != INVALID_SOCKET && !exitRequested_) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(clientSock_, &readfds);

            timeval tv;
            tv.tv_sec  = 0;
            tv.tv_usec = 30 * 1000;

            int ret = select(0, &readfds, nullptr, nullptr, &tv);
            if (ret == SOCKET_ERROR) {
                std::cerr << "select error: " << WSAGetLastError() << "\n";
                break; // 当前 client 会话结束
            }

            if (ret > 0 && FD_ISSET(clientSock_, &readfds)) {
                std::vector<char> body;
                MessageHeader hdrHost{};
                if (!recvOneMessage(body, hdrHost)) {
                    std::cout << "Client disconnected.\n";
                    break; // client 断开 => 回到外层 accept 下一个
                }
                if (!handleMessage(hdrHost, body)) {
                    std::cout << "handleMessage failed.\n";
                    break; // 当前 client 会话结束
                }
            }

            if (previewOn_) {
                ULONGLONG now = GetTickCount64();
                if (now - lastFrameTick >= (ULONGLONG)frameIntervalMs) {
                    if (!sendFrame(/*requestId*/0,
                                   FRAME_TYPE_PREVIEW,
                                   frameSeq_++,
                                   cfg_.previewWidth,
                                   cfg_.previewHeight,
                                   cfg_.previewFmt)) {
                        std::cout << "send preview frame failed.\n";
                        break; // 当前 client 会话结束
                    }
                    lastFrameTick = now;
                }
            }
        }

        // 4) 结束当前 client：只关闭 clientSock，继续等下一个
        if (clientSock_ != INVALID_SOCKET) {
            closesocket(clientSock_);
            clientSock_ = INVALID_SOCKET;
        }

        if (exitRequested_) {
            // exit 命令触发：关闭整个 server
            running_ = false;
            break;
        }

        std::cout << "Waiting for next client...\n";
    }

    // 关闭 listen 并退出
    stop();
    std::cout << "Server exit.\n";
}

void SocketServer::stop() {
    running_ = false;

    if (clientSock_ != INVALID_SOCKET) {
        closesocket(clientSock_);
        clientSock_ = INVALID_SOCKET;
    }
    if (listenSock_ != INVALID_SOCKET) {
        closesocket(listenSock_);
        listenSock_ = INVALID_SOCKET;
    }
}

// ================== winsock ==================
bool SocketServer::initWinsock() {
    if (winsockInited_) return true;

    WSADATA wsaData{};
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r != 0) {
        std::cerr << "WSAStartup failed: " << r << "\n";
        return false;
    }
    winsockInited_ = true;
    return true;
}

void SocketServer::cleanupWinsock() {
    if (winsockInited_) {
        WSACleanup();
        winsockInited_ = false;
    }
}

// ================== listen/accept ==================
bool SocketServer::createListenSocket() {
    listenSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock_ == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << "\n";
        return false;
    }

    int opt = 1;
    setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&opt, sizeof(opt));
    return true;
}

bool SocketServer::bindAndListen() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(cfg_.port);
    addr.sin_addr.s_addr = inet_addr(cfg_.bindIp.c_str());

    if (bind(listenSock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << "\n";
        return false;
    }
    if (listen(listenSock_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << "\n";
        return false;
    }
    return true;
}

bool SocketServer::acceptOneClient() {
    sockaddr_in clientAddr{};
    int clientLen = sizeof(clientAddr);

    clientSock_ = accept(listenSock_, (sockaddr*)&clientAddr, &clientLen);
    if (clientSock_ == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << "\n";
        return false;
    }
    return true;
}

// ================== io helpers ==================
bool SocketServer::recvAll(SOCKET s, char* buf, int len) {
    int received = 0;
    while (received < len) {
        int ret = recv(s, buf + received, len - received, 0);
        if (ret <= 0) return false;
        received += ret;
    }
    return true;
}

bool SocketServer::sendAll(SOCKET s, const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int ret = send(s, buf + sent, len - sent, 0);
        if (ret <= 0) return false;
        sent += ret;
    }
    return true;
}

bool SocketServer::containsCmd(const std::string& json, const std::string& cmd) {
    std::string s = json;
    std::string c = cmd;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);
    return s.find(c) != std::string::npos;
}

// ================== protocol send/recv ==================
bool SocketServer::sendCommandResponse(uint32_t requestId,
                                       const std::string& cmd,
                                       int result,
                                       const std::string& message) {
    std::string json = "{"
        "\"cmd\":\"" + cmd + "\","
        "\"result\":" + std::to_string(result) + ","
        "\"message\":\"" + message + "\""
        "}";

    MessageHeader hdr{};
    hdr.msgType   = htons((uint16_t)MSG_COMMAND_RESPONSE);
    hdr.version   = htons((uint16_t)1);
    hdr.requestId = htonl(requestId);

    uint32_t bodyLen = (uint32_t)(sizeof(hdr) + json.size());
    uint32_t netBodyLen = htonl(bodyLen);

    std::vector<char> buf;
    buf.resize(4 + bodyLen);

    memcpy(buf.data(), &netBodyLen, 4);
    memcpy(buf.data() + 4, &hdr, sizeof(hdr));
    memcpy(buf.data() + 4 + sizeof(hdr), json.data(), json.size());

    return sendAll(clientSock_, buf.data(), (int)buf.size());
}

bool SocketServer::sendFrame(uint32_t requestId,
                             FrameType frameType,
                             uint64_t seq,
                             uint32_t width,
                             uint32_t height,
                             PixelFormat fmt) {
    const uint32_t dummySize = 50 * 1024;
    std::vector<uint8_t> dummyData(dummySize, 0);

    FrameDataHeader fHdrHost{};
    fHdrHost.frameType   = (uint8_t)frameType;
    fHdrHost.seq         = seq;
    fHdrHost.timestampMs = (uint64_t)GetTickCount64();
    fHdrHost.width       = width;
    fHdrHost.height      = height;
    fHdrHost.pixelFormat = (uint32_t)fmt;
    fHdrHost.dataLength  = dummySize;

    FrameDataHeader fHdrNet = fHdrHost;
    fHdrNet.seq         = htonll(fHdrNet.seq);
    fHdrNet.timestampMs = htonll(fHdrNet.timestampMs);
    fHdrNet.width       = htonl(fHdrNet.width);
    fHdrNet.height      = htonl(fHdrNet.height);
    fHdrNet.pixelFormat = htonl(fHdrNet.pixelFormat);
    fHdrNet.dataLength  = htonl(fHdrNet.dataLength);

    MessageHeader msgHdr{};
    msgHdr.msgType   = htons((uint16_t)MSG_FRAME_DATA);
    msgHdr.version   = htons((uint16_t)1);
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

    return sendAll(clientSock_, buf.data(), (int)buf.size());
}

bool SocketServer::recvOneMessage(std::vector<char>& outBody, MessageHeader& hdrHost) {
    uint32_t netBodyLen = 0;
    if (!recvAll(clientSock_, (char*)&netBodyLen, (int)sizeof(netBodyLen))) {
        return false;
    }

    uint32_t bodyLen = ntohl(netBodyLen);
    if (bodyLen < sizeof(MessageHeader)) {
        std::cerr << "Invalid bodyLen: " << bodyLen << "\n";
        return false;
    }

    outBody.resize(bodyLen);
    if (!recvAll(clientSock_, outBody.data(), (int)bodyLen)) {
        return false;
    }

    memcpy(&hdrHost, outBody.data(), sizeof(MessageHeader));
    hdrHost.msgType   = ntohs(hdrHost.msgType);
    hdrHost.version   = ntohs(hdrHost.version);
    hdrHost.requestId = ntohl(hdrHost.requestId);

    return true;
}

// ================== message handler ==================
bool SocketServer::handleMessage(const MessageHeader& hdrHost,
                                 const std::vector<char>& body) {
    if (body.size() < sizeof(MessageHeader)) {
        std::cerr << "Invalid body size: " << body.size() << "\n";
        return false;
    }

    const char* payload = body.data() + sizeof(MessageHeader);
    int payloadLen = (int)(body.size() - sizeof(MessageHeader));

    switch (hdrHost.msgType) {
    case MSG_COMMAND_REQUEST: {
        std::string json(payload, payloadLen);
        std::cout << "[REQ] requestId=" << hdrHost.requestId
                  << " json=" << json << "\n";

        if (containsCmd(json, "\"cmd\":\"opencamera\"")) {
            return sendCommandResponse(hdrHost.requestId, "openCamera", 0, "OK");
        } else if (containsCmd(json, "\"cmd\":\"closecamera\"")) {
            return sendCommandResponse(hdrHost.requestId, "closeCamera", 0, "OK");
        } else if (containsCmd(json, "\"cmd\":\"startpreview\"")) {
            previewOn_ = true;
            frameSeq_ = 1;
            return sendCommandResponse(hdrHost.requestId, "startPreview", 0, "OK");
        } else if (containsCmd(json, "\"cmd\":\"stoppreview\"")) {
            previewOn_ = false;
            return sendCommandResponse(hdrHost.requestId, "stopPreview", 0, "OK");
        } else if (containsCmd(json, "\"cmd\":\"getpreviewframe\"")) {
            if (!sendCommandResponse(hdrHost.requestId, "getPreviewFrame", 0, "OK"))
                return false;
            return sendFrame(hdrHost.requestId, FRAME_TYPE_SINGLE,
                             frameSeq_++,
                             cfg_.previewWidth, cfg_.previewHeight, cfg_.previewFmt);
        } else if (containsCmd(json, "\"cmd\":\"capture\"")) {
            if (!sendCommandResponse(hdrHost.requestId, "capture", 0, "OK"))
                return false;
            return sendFrame(hdrHost.requestId, FRAME_TYPE_CAPTURE,
                             frameSeq_++,
                             cfg_.previewWidth, cfg_.previewHeight, cfg_.previewFmt);
        } else {
            return sendCommandResponse(hdrHost.requestId, "unknown", -1, "Unknown cmd");
        }
    }

    case MSG_HEARTBEAT:
        std::cout << "[HEARTBEAT] requestId=" << hdrHost.requestId << "\n";
        return true;

    default:
        std::cout << "[UNKNOWN MSG] type=" << hdrHost.msgType
                  << " len=" << body.size() << "\n";
        return true;
    }
}
