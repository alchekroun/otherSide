#pragma once
#include <functional>
#include "app/AppEvent.h"

namespace otherside {

class IScreen {
public:
    using EventSink = std::function<void(AppEvent)>;

    explicit IScreen(EventSink sink) : _eventSink(std::move(sink)) {}
    virtual ~IScreen() = default;

    virtual void render() = 0;

protected:
    EventSink _eventSink;
};

}
