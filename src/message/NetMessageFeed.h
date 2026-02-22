#pragma once

#include <mutex>
#include <queue>

#include "NetMessage.h"

namespace otherside
{

class IMessageFeed
{
public:
    virtual ~IMessageFeed() = default;
    virtual bool empty() = 0;
    virtual std::vector<UiMessage> consume() = 0;
};

class UiMessageFeed final : public IMessageFeed
{
public:
    void push(const UiMessage &msg)
    {
        std::scoped_lock lock(_mtx);
        _buffer.push_back(msg);
    }

    std::vector<UiMessage> consume() override
    {
        std::scoped_lock lock(_mtx);
        auto out = std::move(_buffer);
        _buffer.clear();
        return out;
    }

    bool empty() override
    {
        std::scoped_lock lock(_mtx);
        return _buffer.empty();
    }

private:
    std::mutex _mtx;
    std::vector<UiMessage> _buffer;
};

} // namespace otherside
