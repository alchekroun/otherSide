#pragma once

namespace otherside {

class IPanel {
public:
    virtual ~IPanel() = default;

    virtual void render() = 0;
};

}