#pragma once

#include <thread>
#include <atomic>

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

}
