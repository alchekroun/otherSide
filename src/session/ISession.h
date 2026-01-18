#pragma once

#include <string>

#include "message/NetMessage.h"

namespace otherside {

class SessionThreaded {
protected:
    std::thread _thread;
    std::atomic<bool> _running{false};

public:
    virtual ~SessionThreaded() {
        stop();
    }

    void startThread() {
        _running = true;
        _thread = std::thread([this]() { run(); });
    }

    void stop() {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

protected:
    virtual void run() = 0;
};

class ISession : public SessionThreaded {
public:
    virtual ~ISession() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update(float dt) = 0;
};

class ISessionControl {
public:
    virtual void sendMessage(UiMessage msg) = 0;
    // virtual void ping() = 0;
    // virtual void sendText(std::string_view) = 0;
};

}
