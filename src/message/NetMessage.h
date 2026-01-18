#pragma once

#include <string>
#include <vector>

#include "DCMessageType.h"

namespace otherside {

enum class PeerId {
    HOST,
    CLIENT
};

using TimestampMs = uint64_t;

struct UiMessage {
    DCMessageType type;
    PeerId from;
    TimestampMs createdTs;
    TimestampMs receivedTs;
    std::string text;
};

TimestampMs nowMs();
std::vector<uint8_t> serialize(const UiMessage& msg);
UiMessage deserialize(const std::vector<std::byte>& buf);

}
