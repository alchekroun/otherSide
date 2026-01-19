#pragma once

namespace otherside
{

enum class MsgType : std::uint8_t
{
    PING,
    ACK_CONNECTION,
    REQUEST,
    OFFER,
    READY,
    ANSWER
};

} // namespace otherside
