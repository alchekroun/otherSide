#pragma once

enum class MsgType : uint32_t
{
    PING,
    ACK_CONNECTION,
    OFFER,
    READY,
    ANSWER
};