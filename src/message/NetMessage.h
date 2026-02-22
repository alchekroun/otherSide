#pragma once

#include <string>
#include <vector>

#include "DCMessageType.h"
#include "utils.h"

namespace otherside
{

enum class PeerId : std::uint8_t
{
    HOST,
    CLIENT
};

struct UiMessage
{
    DCMessageType type;
    PeerId from;
    utils::TimestampMs createdTs;
    utils::TimestampMs receivedTs;
    std::string text;
};

std::vector<uint8_t> serialize(const UiMessage &msg);
UiMessage deserialize(const std::vector<std::byte> &buf);

} // namespace otherside
