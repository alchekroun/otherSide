#pragma once

#include <string>
#include <queue>
#include <mutex>

namespace otherside {

struct UiMessage {
    std::string from;
    std::string text;
    // uint64_t timestamp;
};

class IMessageFeed {
public:
    virtual std::vector<UiMessage> consume() = 0;
    virtual ~IMessageFeed() = default;
};

class UiMessageFeed final : public IMessageFeed {
public:
    void push(UiMessage msg) {
        std::lock_guard lock(_mtx);
        _buffer.push_back(std::move(msg));
    }

    std::vector<UiMessage> consume() override {
        std::lock_guard lock(_mtx);
        auto out = std::move(_buffer);
        _buffer.clear();
        return out;
    }

private:
    std::mutex _mtx;
    std::vector<UiMessage> _buffer;
};

}
