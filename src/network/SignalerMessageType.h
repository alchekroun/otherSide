#pragma once

namespace otherside
{

enum class MsgType : std::uint32_t
{
    PING,
    ACK_CONNECTION,
    REQUEST,
    OFFER,
    READY,
    ANSWER
};

} // namespace otherside
