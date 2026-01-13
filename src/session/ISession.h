#pragma once

#include <string>

namespace otherside {

class ISession {
    public:
    virtual ~ISession() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update(float dt) = 0;

    virtual std::string statusText() const = 0;
};

}
