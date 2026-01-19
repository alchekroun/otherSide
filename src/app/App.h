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
          _rxMessagesFeed(std::make_unique<UiMessageFeed>()), _txMessageFeed(std::make_unique<UiMessageFeed>())
    {
        _gui->registerScreen<RoleSelectionScreen>(AppScreen::RoleSelectionScreen);
        _gui->registerScreen<IdleScreen>(AppScreen::IdleScreen);
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

} // namespace otherside