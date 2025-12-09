// protocol_common.h
#pragma once

#include <stdint.h>

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

// 消息类型
enum MsgType : uint16_t {
    MSG_COMMAND_REQUEST = 1,
    MSG_COMMAND_RESPONSE = 2,
    MSG_FRAME_DATA = 3,
    MSG_EVENT_NOTIFICATION = 4,
    MSG_HEARTBEAT = 5
};

// 像素格式
enum PixelFormat : uint32_t {
    PIX_FMT_JPEG = 1,
    PIX_FMT_NV12 = 2,
    PIX_FMT_RGB24 = 3,
    PIX_FMT_H264 = 4
};

// 帧类型
enum FrameType : uint8_t {
    FRAME_TYPE_PREVIEW = 1,
    FRAME_TYPE_CAPTURE = 2,
    FRAME_TYPE_SINGLE = 3
};

// 通用消息头（在 messageBody 的最前面）
struct MessageHeader
{
    uint16_t msgType;    // MsgType
    uint16_t version;    // 协议版本，目前固定 1
    uint32_t requestId;  // 请求ID/关联ID
};

// FrameData 的二进制头（不含真正图像数据）
struct FrameDataHeader
{
    uint8_t  frameType;    // FrameType
    uint8_t  reserved1;
    uint16_t reserved2;
    uint64_t seq;
    uint64_t timestampMs;
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormat;  // PixelFormat
    uint32_t dataLength;   // 后面紧跟的数据长度
};
