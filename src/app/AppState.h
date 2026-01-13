#pragma once

namespace otherside {

enum class AppState {
    Boot,
    Idle,        // UI shown, no session
    StartingHost,
    StartingClient,
    Hosting,
    Connected,
    ShuttingDown
};

}
