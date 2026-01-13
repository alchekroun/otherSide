#pragma once

#include <memory>
#include <thread>

#include "logger/Logger.h"
#include "ui/BaseUI.h"
#include "app/AppEvent.h"
#include "app/AppState.h"
#include "session/ISession.h"
#include "session/HostSession.h"
#include "session/ClientSession.h"

namespace otherside {

// template<typename T>
class Application {

    public:
    Application() {
        _gui = std::make_unique<BaseUI>(
            [this](AppEvent ev) { pushEvent(ev); }
        );
    }
    ~Application() { stop(); }

    void pushEvent(AppEvent ev) {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _eventQueue.push(ev);
    }

    void uptate() {
        while (auto ev = popEvent()) {
            handleEvent(*ev);
        }
    }

    void handleEvent(AppEvent ev) {
        switch (ev)
        {
        case AppEvent::StartHost:
            _log->msg("Switching to HOST mode");
            setState(AppState::StartingHost);
            break;
        case AppEvent::StartClient:
            _log->msg("Switching to Client mode");
            setState(AppState::StartingClient);
            break;
        default:
            break;
        }
    }

    void start() {
        SetConfigFlags(FLAG_WINDOW_HIGHDPI);
        InitWindow(_WINDOW_HEIGHT, _WINDOW_WIDTH, _WINDOW_HEADER_TEXT.c_str());
        SetWindowState(FLAG_VSYNC_HINT);

        _gui->start();

        while (!WindowShouldClose())
        {
            float deltaTime = GetFrameTime();

            // IO SOMEHOW ?

            // Update
            uptate();

            // Drawing
            BeginDrawing();
            ClearBackground(RAYWHITE);

            _gui->render();

            DrawFPS(GetScreenWidth() - 95, 10);
            EndDrawing();
        }

    }

    void stop() {
        _log->msg("Stopping...");
        if (_gui) {
            _gui->stop();
            _gui.reset();
        }
        if (_session) {
            _session->stop();
            _session.reset();
        }
        CloseWindow();
    }

    std::unique_ptr<BaseUI> _gui;
    std::unique_ptr<ISession> _session;
    // std::unique_ptr<T> _network = std::make_unique<T>();

    private:
    std::optional<AppEvent> popEvent() {
        std::lock_guard<std::mutex> lock(_eventMutex);
        if (_eventQueue.empty()) {
            return std::nullopt;
        }

        auto ev = _eventQueue.front();
        _eventQueue.pop();
        return ev;
    }

    void setState(AppState newState) {
        if (_state == newState) {
            return;
        }

        _log->msg("State change : ", static_cast<int>(_state), " -> ", static_cast<int>(newState));

        switch (newState)
        {
        case AppState::StartingHost:
            _log->msg("Switching to HOST mode");
            _session = std::make_unique<HostSession>(8000);
            _session->start();
            setState(AppState::Hosting);
            break;
        case AppState::StartingClient:
            _log->msg("Switching to Client mode");
            _session = std::make_unique<ClientSession>("127.0.0.1", 8000);
            _session->start();
            setState(AppState::Connected);
            break;
        case AppState::ShuttingDown:
            _log->msg("Shutting down");
            stop();
        default:
            break;
        }
    }


    std::queue<AppEvent> _eventQueue;
    std::mutex _eventMutex;

    AppState _state = AppState::Boot;

    std::thread _mainThread;
    inline static int _FPS = 24;
    inline static int _WINDOW_HEIGHT = 800;
    inline static int _WINDOW_WIDTH = 600;
    inline static std::string _WINDOW_HEADER_TEXT = "otherSide - test zone";
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("App");
};

}