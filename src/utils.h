#pragma once

#include <memory>

namespace otherside::utils
{

template <class T> std::weak_ptr<T> make_weak_ptr(const std::shared_ptr<T> &ptr)
{
    return ptr;
}

using TimestampMs = uint64_t;

inline TimestampMs nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

} // namespace otherside::utils