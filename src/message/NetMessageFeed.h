#pragma once

#include <queue>
#include <mutex>

#include "NetMessage.h"

namespace otherside {

class IMessageFeed {
public:
    virtual ~IMessageFeed() = default;
    virtual bool empty() = 0;
    virtual std::vector<UiMessage> consume() = 0;
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

    bool empty() override {
        std::lock_guard lock(_mtx);
        return _buffer.empty();
    }

private:
    std::mutex _mtx;
    std::vector<UiMessage> _buffer;
};

}
