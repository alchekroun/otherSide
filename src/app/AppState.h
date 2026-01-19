#pragma once

namespace otherside
{

enum class AppState : std::uint8_t
{
    BOOT,
    IDLE, // UI shown, no session
    STARTING_HOST,
    STARTING_CLIENT,
    HOSTING,
    CONNECTED,
    SHUTTING_DOWN
};

} // namespace otherside
