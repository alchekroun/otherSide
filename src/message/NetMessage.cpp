#include "NetMessage.h"

namespace otherside
{

std::vector<uint8_t> serialize(const UiMessage &msg)
{
    std::vector<uint8_t> buf;

    auto push = [&](auto value) {
        uint8_t *p = reinterpret_cast<uint8_t *>(&value);
        buf.insert(buf.end(), p, p + sizeof(value));
    };

    push(msg.type);
    push(msg.from);
    push(msg.createdTs);
    push(msg.receivedTs);

    uint32_t len = static_cast<uint32_t>(msg.text.size());
    push(len);
    buf.insert(buf.end(), msg.text.begin(), msg.text.end());

    return buf;
}

UiMessage deserialize(const std::vector<std::byte> &buf)
{
    size_t offset = 0;

    auto read = [&](auto &value) {
        std::memcpy(&value, buf.data() + offset, sizeof(value));
        offset += sizeof(value);
    };

    UiMessage msg;
    read(msg.type);
    read(msg.from);
    read(msg.createdTs);
    read(msg.receivedTs);

    uint32_t len;
    read(len);

    msg.text.assign(reinterpret_cast<const char *>(buf.data() + offset), len);

    return msg;
}

} // namespace otherside
