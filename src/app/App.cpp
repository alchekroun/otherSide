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
    std::lock_guard<std::mutex> lock(_eventMutex);
    _eventQueue.push(ev);
}

void Application::handleEvent(AppEvent ev)
{
    switch (ev)
    {
    case AppEvent::StartHost:
        _log->msg("Switching to HOST mode");
        setState(AppState::StartingHost);
        _gui->setScreen(AppScreen::HostScreen);
        break;
    case AppEvent::StartClient:
        _log->msg("Switching to Client mode");
        setState(AppState::StartingClient);
        _gui->setScreen(AppScreen::ClientScreen);
        break;
    default:
        break;
    }
}

void Application::start()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(_WINDOW_HEIGHT, _WINDOW_WIDTH, _WINDOW_HEADER_TEXT.c_str());
    SetWindowState(FLAG_VSYNC_HINT);

    _gui->setScreen(AppScreen::IdleScreen);
    _gui->start();

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

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
    std::lock_guard<std::mutex> lock(_eventMutex);
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
    _session = std::make_unique<HostSession>(8000, _rxMessagesFeed, _txMessageFeed);
    _session->start();
    _gui->registerScreen<HostScreen>(AppScreen::HostScreen, _rxMessagesFeed, _txMessageFeed);

    setState(AppState::Hosting);
}
void Application::initClientMode()
{
    _session = std::make_unique<ClientSession>("127.0.0.1", 8000, _rxMessagesFeed, _txMessageFeed);
    _session->start();
    _gui->registerScreen<ClientScreen>(AppScreen::ClientScreen, _rxMessagesFeed, _txMessageFeed);

    setState(AppState::Connected);
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
    case AppState::StartingHost:
        _log->msg("Switching to HOST mode");
        initHostMode();
        break;
    case AppState::StartingClient:
        _log->msg("Switching to Client mode");
        initClientMode();
        break;
    case AppState::ShuttingDown:
        _log->msg("Shutting down");
        stop();
    default:
        break;
    }
}

} // namespace otherside