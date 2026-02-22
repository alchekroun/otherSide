#pragma once

#include <memory>
#include <thread>

#include "app/AppEvent.h"
#include "app/AppScreen.h"
#include "app/AppState.h"
#include "logger/Logger.h"
#include "session/ClientSession.h"
#include "session/HostSession.h"
#include "session/ISession.h"
#include "ui/ScreenRouter.h"

namespace otherside
{

class Application
{
    using EventSink = std::function<void(AppEvent)>;

public:
    Application()
        : _gui(std::make_unique<ScreenRouter>([this](AppEvent ev) { pushEvent(ev); })),
          _rxMessagesFeed(std::make_shared<UiMessageFeed>()),
          _txMessageFeed(std::make_shared<UiMessageFeed>()),
          _rxFrameFeed(std::make_shared<FrameFeed>())
    {
        _gui->registerScreen<RoleSelectionScreen>(AppScreen::ROLE_SELECTION);
        _gui->registerScreen<IdleScreen>(AppScreen::IDLE);
    }
    ~Application()
    {
        stop();
    }

    void pushEvent(AppEvent ev);
    void handleEvent(AppEvent ev);

    void update();
    void start();
    void stop();

private:
    void initHostMode();
    void initClientMode();

    std::optional<AppEvent> popEvent();
    void setState(AppState newState);

    std::shared_ptr<UiMessageFeed> _rxMessagesFeed;
    std::shared_ptr<UiMessageFeed> _txMessageFeed;
    std::unique_ptr<ScreenRouter> _gui;
    std::unique_ptr<ISession> _session;

    std::shared_ptr<FrameFeed> _rxFrameFeed;

    std::queue<AppEvent> _eventQueue;
    std::mutex _eventMutex;

    AppState _state = AppState::BOOT;

    std::thread _mainThread;
    inline static int _fps = 24;
    inline static int _window_height = 800;
    inline static int _window_width = 600;
    inline static std::string _window_header_text = "otherSide - test zone";
    std::unique_ptr<Logger> _log = std::make_unique<Logger>("App");
};

} // namespace otherside