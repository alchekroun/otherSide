#pragma once

#include <imgui.h>
#include <rlImGui.h>
#include <memory>
#include <functional>

#include "logger/Logger.h"
#include "app/AppEvent.h"
#include "app/AppScreen.h"
#include "screen/IScreen.h"
#include "screen/RoleSelectionScreen.h"
#include "screen/HostScreen.h"
#include "screen/ClientScreen.h"
#include "screen/IdleScreen.h"

namespace otherside {
class ScreenRouter {
    public:
    using EventSink = std::function<void(AppEvent)>;

    explicit ScreenRouter(EventSink sink) : _eventSink(std::move(sink))
    {}

    void start() {
        IMGUI_CHECKVERSION();
        rlImGuiSetup(false);
        setScreen(AppScreen::RoleSelectionScreen);
    };

    void stop() {
        rlImGuiShutdown();
    }

    template<typename T, typename... Args>
    void registerScreen(AppScreen screen, Args&&... args) {
        _screens[screen] = std::make_unique<T>(_eventSink, std::forward<Args>(args)...);
    }

    void setScreen(AppScreen screen) {
        _current = screen;
    }

    void render() {
        rlImGuiBegin();
        if (_screens.count(_current)) _screens[_current]->render();
        rlImGuiEnd();
    }

    private:
    std::vector<std::string> _displayMessages;

    EventSink _eventSink;
    AppScreen _current;
    std::unordered_map<AppScreen, std::unique_ptr<IScreen>> _screens;
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("ScreenRouter");
};
}