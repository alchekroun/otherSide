#pragma once

#include <imgui.h>
#include <rlImGui.h>
#include <memory>
#include <functional>

#include "logger/Logger.h"
#include "app/AppEvent.h"

namespace otherside {
class BaseUI {
    public:
    using EventSink = std::function<void(AppEvent)>;

    explicit BaseUI(EventSink sink) : _eventSink(std::move(sink)) {}

    void start() {
        _log->msg("Starting");
        IMGUI_CHECKVERSION();
        rlImGuiSetup(false);
    };
    void stop() {
        rlImGuiShutdown();
    }

    void render() {
        rlImGuiBegin();
        {
            ImGui::Begin("OtherSide");

            ImGui::Text("Chose your role : ");

            if (ImGui::Button("Host")) {
                _mode = Mode::Host;
                _eventSink(AppEvent::StartHost);
            }

            ImGui::SameLine();

            if (ImGui::Button("Client")) {
                _mode = Mode::Client;
                _eventSink(AppEvent::StartClient);
            }

            ImGui::Separator();

            // Persistent text based on state
            switch (_mode) {
                case Mode::Host:
                    ImGui::TextColored(ImVec4(0,1,0,1), "Mode: HOST");
                    break;
                case Mode::Client:
                    ImGui::TextColored(ImVec4(0,0.7f,1,1), "Mode: CLIENT");
                    break;
                default:
                    ImGui::TextDisabled("No mode selected");
                    break;
            }
            ImGui::End();
        }
        rlImGuiEnd();
    }

    private:
    enum class Mode {
        None,
        Host,
        Client
    };

    Mode _mode = Mode::None;

    EventSink _eventSink;

    std::unique_ptr<Logger> _log = std::make_unique<Logger>("BaseUI");
};
}