#include "SocketServer.h"
#include <ws2tcpip.h>
#include <iostream>
#include <algorithm>
#include <cstring>


#include "CanonEDSCamera.h"
#include "AigoCamera.h"



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
//static inline uint64_t htonll(uint64_t x) { return bswap64(x); }
//static inline uint64_t ntohll(uint64_t x) { return bswap64(x); }

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


        // 3) 服务当前 client，直到断开或 exit 命令
        ULONGLONG nextFrameTick = GetTickCount64(); // 下一帧目标时间点
        const double frameIntervalMs = 1000.0 / cfg_.previewFps;
        while (running_ && clientSock_ != INVALID_SOCKET && !exitRequested_) {

            // 1) 计算 select 超时：有预览就等到下一帧；没预览就“无限等消息”
            timeval* ptv = nullptr;
            timeval tv{};

            if (previewOn_) {
                ULONGLONG now = GetTickCount64();
                long waitMs = 0;

                if (now < nextFrameTick) {
                    waitMs = (long)(nextFrameTick - now);
                }
                else {
                    waitMs = 0; // 已经过点了，立刻醒来发帧（或先处理消息也行）
                }

                tv.tv_sec = waitMs / 1000;
                tv.tv_usec = (waitMs % 1000) * 1000;
                ptv = &tv;
            }
            else {
                // 没开预览：不需要定时醒来，直接阻塞等消息（CPU 不空转）
                ptv = nullptr;
            }

            // 2) 等消息（或等到发帧时间到）
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(clientSock_, &readfds);

            int ret = select(0, &readfds, nullptr, nullptr, ptv);
            if (ret == SOCKET_ERROR) {
                std::cerr << "select error: " << WSAGetLastError() << "\n";
                break;
            }

            // 3) 有消息就处理（不会要求客户端持续发）
            if (ret > 0 && FD_ISSET(clientSock_, &readfds)) {
                std::vector<char> body;
                MessageHeader hdrHost{};
                if (!recvOneMessage(body, hdrHost)) {
                    std::cout << "Client disconnected.\n";
                    break;
                }
                if (!handleMessage(hdrHost, body)) {
                    std::cout << "handleMessage failed.\n";
                    break;
                }
                // handleMessage 里可能会把 previewOn_ 打开/关闭
                // 如果刚打开预览，可以重置 nextFrameTick：
                //nextFrameTick = GetTickCount64();
            }

            // 4) 时间到就发预览帧（即使没有收到任何消息）
            if (previewOn_) {
                ULONGLONG now = GetTickCount64();
                if (now >= nextFrameTick) {

                    if (!sendFrame(/*requestId*/0,
                        FRAME_TYPE_PREVIEW,
                        frameSeq_++,
                        cfg_.previewWidth,
                        cfg_.previewHeight,
                        cfg_.previewFmt)) {
                        std::cout << "send preview frame failed.\n";
                        // 这里你要不要 break 取决于你希望“发送失败是否断开”
                    }
                    pCamera_->PushGetEvent();

                    // 计算下一帧时间点：用“加间隔”的方式比 now 更稳定（少抖动）
                    nextFrameTick += (ULONGLONG)frameIntervalMs;

                    // 如果卡太久导致 nextFrameTick 落后很多，做一下追赶，避免瞬间狂发：
                    if (nextFrameTick + (ULONGLONG)frameIntervalMs < now) {
                        nextFrameTick = now + (ULONGLONG)frameIntervalMs;
                    }
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

// ================== protocol send/recv ==================
bool SocketServer::sendCommandResponse(uint32_t requestId,const std::string& cmd,const std::string& result, const std::vector<std::string>* param)
{
    json j;
    j["cmd"] = cmd;
    j["result"] = result;
    if(param)
        j["created_files"] = *param;
    std::string str = j.dump();
    std::cout << "[RES] requestId=" << requestId << " json=" << str << "\n";

    MessageHeader hdr{};
    hdr.msgType   = htons((uint16_t)MSG_COMMAND_RESPONSE);
    hdr.version   = htons((uint16_t)1);
    hdr.requestId = htonl(requestId);

    uint32_t bodyLen = (uint32_t)(sizeof(hdr) + str.size());
    uint32_t netBodyLen = htonl(bodyLen);

    std::vector<char> buf;
    buf.resize(4 + bodyLen);

    memcpy(buf.data(), &netBodyLen, 4);
    memcpy(buf.data() + 4, &hdr, sizeof(hdr));
    memcpy(buf.data() + 4 + sizeof(hdr), str.data(), str.size());

    return sendAll(clientSock_, buf.data(), (int)buf.size());
}

bool SocketServer::sendFrame(uint32_t requestId,
                             FrameType frameType,
                             uint64_t seq,
                             uint32_t width,
                             uint32_t height,
                             PixelFormat fmt) 
{
    if (pCamera_->IsPreviewStop() || pCamera_->IsPausePreview())
    {
        LOG(L"Camera is not in preview");
        return false;
    }
        
    std::vector<uint8_t> dummyData = pCamera_->GetFrame2();
    const uint32_t dummySize = dummyData.size();
    if (dummySize == 0)
    {
        LOG(L"dummySize is 0");
        return false;
    }

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

camera_param SocketServer::parse_camera_param(const json& j)
{
    camera_param p;

    // value(key, default) → key 不存在直接用 default
    p.iso = j.value("iso", 0);
    p.exposure = j.value("exposuretime", 0);
    p.aperture = j.value("aperture", 0);
    p.wb = j.value("wb", 0);

    return p;
}
bool SocketServer::parseCaptureSettingFromJson(const json& j,CaptureSetting& outSetting)
{
    // --- 通用参数 ---
    if(j.contains("ImageSize"))
        outSetting.setImgSize(j.value("ImageSize", 0));
    if (j.contains("ImageQuality"))
        outSetting.setImgQuality(j.value("ImageQuality", 0));

    // --- 各通道 ---
    if (j.contains("RGB") && j["RGB"].is_object()) {
        outSetting.set_rgb(parse_camera_param(j["RGB"]));
    }

    if (j.contains("UV") && j["UV"].is_object()) {
        outSetting.set_uv(parse_camera_param(j["UV"]));
    }

    if (j.contains("PL") && j["PL"].is_object()) {
        outSetting.set_pl(parse_camera_param(j["PL"]));
    }

    if (j.contains("NPL") && j["NPL"].is_object()) {
        outSetting.set_npl(parse_camera_param(j["NPL"]));
    }

    return true;
}

bool SocketServer::parseCaptureSettingFromJson(const json& j, CaptureSetting2& outSetting)
{
    // --- 通用参数 ---
    if (j.contains("ImageSize"))
        outSetting.setImgSize(j.value("ImageSize", 0));
    if (j.contains("ImageQuality"))
        outSetting.setImgQuality(j.value("ImageQuality", 0));

    if (j.contains("capture_type")) {
        outSetting.setCaptureType(j.value("capture_type", ""));
    }

    if (j.contains("params") && j["params"].is_object()) {
        outSetting.set_param(parse_camera_param(j["params"]));
    }
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
        std::cout << "[REQ] requestId=" << hdrHost.requestId
                  << " json=" << std::string(payload, payloadLen) << "\n";

        json j = nlohmann::json::parse(std::string(payload, payloadLen));
        std::string cmd = j.value("cmd", "");
        std::string result = "NG";
        if (cmd == "open")
        {
            int series = j.value("series", NoneType);
            if(series== CanonEOS)
                pCamera_ = std::make_unique<CanonEDSCamera>();
            else if (series == CanonEOS)
                pCamera_ = std::make_unique<CAigoCamera>();
            std::wstring dllname = Util::Instance().AnsiToWString(j.value("dllname", "").c_str());
            if(pCamera_->Init(dllname.c_str())==0)
                result ="OK";
            else
                result = "NG";
        }
        else if (cmd == "close")
        {
            previewOn_ = false;
            pCamera_->unInit();
            result = "OK";
        }
        else if (cmd == "startpreview")
        {
            if (pCamera_->StartPreview())
            {
                previewOn_ = true;
                result = "OK";
            }
        }
        else if (cmd == "stoppreview")
        {
            pCamera_->StopPreview();
            previewOn_ = false;
            result = "OK";
        }
        else if (cmd == "preview_setting")
        {
            if (pCamera_->IsInited())
            {
                CaptureSetting2 settings;
                if (parseCaptureSettingFromJson(j, settings))
                {
                    pCamera_->SetImgQuality(settings.getImgQuality());
                    pCamera_->SetImgSize(settings.getImgSize());
                    pCamera_->SetISO(settings.get_param().iso);
                    pCamera_->SetExposure(settings.get_param().exposure);
                    pCamera_->SetAperture(settings.get_param().aperture);
                    pCamera_->SetWB(settings.get_param().wb);
                    result = "OK";
                }

            }
        }
        else if (cmd == "capture_setting")
        {
            if (pCamera_->IsInited())
            {
                CaptureSetting2 settings;
                if (parseCaptureSettingFromJson(j, settings))
                {
                    pCamera_->SetISO(settings.get_param().iso);
                    pCamera_->SetExposure(settings.get_param().exposure);
                    pCamera_->SetAperture(settings.get_param().aperture);
                    pCamera_->SetWB(settings.get_param().wb);
                    result = "OK";
                }
            }
        }
        else if (cmd == "online_setting")
        {
            if (pCamera_->IsInited() && !pCamera_->IsPreviewStop() )
            {
                int value = j.value("value", -1);
                if (value > 0)
                {
                    std::string key = j.value("key", "");
                    if (key == "iso")
                    {
                        pCamera_->SetISO(value);
                        result = "OK";
                    }
                    else if (key == "exposuretime")
                    {
                        pCamera_->SetExposure(value);
                        result = "OK";
                    }
                    else if (key == "aperture")
                    {
                        pCamera_->SetAperture(value);
                        result = "OK";
                    }
                    else if (key == "wb")
                    {
                        pCamera_->SetWB(value);
                        result = "OK";
                    }
                    else if (key == "wb")
                    {
                        pCamera_->SetWB(value);
                        result = "OK";
                    }
                    else if (key == "ImageSize")
                    {
                        pCamera_->SetImgSize(value);
                        result = "OK";
                    }
                    else if (key == "ImageQuality")
                    {
                        pCamera_->SetImgQuality(value);
                        result = "OK";
                    }
                }
            }
        }
        else if (cmd == "capture")
        {
            if (pCamera_->IsInited() && !pCamera_->IsPreviewStop())
            {
                std::string save_folder = j.value("save_folder", "");
                
                pCamera_->SetCaptureFolder(save_folder);
                int nID = j.value("capture_id", 0);
                
                pCamera_->SetCaptureID(nID);
                std::string capture_type = j.value("capture_type", "");
                if(capture_type=="RGB")
                    pCamera_->SetILLType(ILL_RGB_TYPE);
                else if (capture_type == "UV")
                    pCamera_->SetILLType(ILL_365UV_TYPE);
                else if (capture_type == "PL")
                    pCamera_->SetILLType(ILL_PL_TYPE);
                else if (capture_type == "NPL")
                    pCamera_->SetILLType(ILL_NPL_TYPE);

                if(0==pCamera_->Capture(hdrHost.requestId, [this](const uint32_t requestId,const std::vector<std::string>& files)
                    {
                         OnCaptureDone(requestId, files);
                    }))
                       return true;
            }
        }
        else if (cmd == "exit")
        {
            //pCamera_->StopPreview();
            pCamera_->unInit(true);
            result = "OK";
            sendCommandResponse(hdrHost.requestId, cmd, result);
            exitRequested_ = true;
            return true;
        }
        return sendCommandResponse(hdrHost.requestId, cmd,result);
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

void SocketServer::OnCaptureDone(const uint32_t requestId , const std::vector<std::string>& createdfiles)
{
    const std::string cmd = "capture";
    const std::string result = "OK";
    sendCommandResponse(requestId, cmd, result, &createdfiles);

}