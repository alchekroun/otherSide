#pragma once

#include "../Common.h"

namespace otherSide {
    inline void log(const std::string& msg) {
        std::cout << "[otherSide]" << msg << std::endl;
    }
}