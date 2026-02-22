#include "App.h"

namespace otherside
{

void Application::update()
{
    while (auto ev = popEvent())
    {
        handleEvent(*ev);
    }
}

void Application::pushEvent(AppEvent ev)
{
    std::scoped_lock<std::mutex> lock(_eventMutex);
    _eventQueue.push(ev);
}

void Application::handleEvent(AppEvent ev)
{
    switch (ev)
    {
    case AppEvent::START_HOST:
        _log->msg("Switching to HOST mode");
        setState(AppState::STARTING_HOST);
        _gui->setScreen(AppScreen::HOST);
        break;
    case AppEvent::START_CLIENT:
        _log->msg("Switching to Client mode");
        setState(AppState::STARTING_CLIENT);
        _gui->setScreen(AppScreen::CLIENT);
        break;
    default:
        break;
    }
}

void Application::start()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(_window_width, _window_height, _window_header_text.c_str());
    SetWindowState(FLAG_VSYNC_HINT);

    _gui->setScreen(AppScreen::IDLE);
    _gui->start();

    while (!WindowShouldClose())
    {
        // IO SOMEHOW ?

        // Update
        update();

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        _gui->render();

        DrawFPS(GetScreenWidth() - 95, 10);
        EndDrawing();
    }
}

void Application::stop()
{
    _log->msg("Stopping...");
    if (_gui)
    {
        _gui->stop();
        _gui.reset();
    }
    if (_session)
    {
        _session->stop();
        _session.reset();
    }
    CloseWindow();
}

std::optional<AppEvent> Application::popEvent()
{
    std::scoped_lock<std::mutex> lock(_eventMutex);
    if (_eventQueue.empty())
    {
        return std::nullopt;
    }

    auto ev = _eventQueue.front();
    _eventQueue.pop();
    return ev;
}

void Application::initHostMode()
{
    _session = std::make_unique<HostSession>(8000, _rxMessagesFeed, _txMessageFeed, _rxFrameFeed);
    _session->start();
    _gui->registerScreen<HostScreen>(AppScreen::HOST, _rxMessagesFeed, _txMessageFeed,
                                     _rxFrameFeed);

    setState(AppState::HOSTING);
}
void Application::initClientMode()
{
    _session = std::make_unique<ClientSession>("127.0.0.1", 8000, _rxMessagesFeed, _txMessageFeed,
                                               _rxFrameFeed);
    _session->start();
    _gui->registerScreen<ClientScreen>(AppScreen::CLIENT, _rxMessagesFeed, _txMessageFeed,
                                       _rxFrameFeed);

    setState(AppState::CONNECTED);
}

void Application::setState(AppState newState)
{
    if (_state == newState)
    {
        return;
    }

    _log->msg("State change : ", static_cast<int>(_state), " -> ", static_cast<int>(newState));

    switch (newState)
    {
    case AppState::STARTING_HOST:
        _log->msg("Switching to HOST mode");
        initHostMode();
        break;
    case AppState::STARTING_CLIENT:
        _log->msg("Switching to Client mode");
        initClientMode();
        break;
    case AppState::SHUTTING_DOWN:
        _log->msg("Shutting down");
        stop();
    default:
        break;
    }
}

} // namespace otherside