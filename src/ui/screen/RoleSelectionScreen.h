// ui/screens/RoleSelectionScreen.h
#pragma once
#include "IScreen.h"
#include <imgui.h>

namespace otherside {

class RoleSelectionScreen : public IScreen {
public:
    RoleSelectionScreen(EventSink sink) : IScreen(sink) {}

    void render() override {
        ImGui::Begin("OtherSide");

        ImGui::Text("Choose your role:");

        if (ImGui::Button("Host")) {
            _eventSink(AppEvent::StartHost);
        }

        ImGui::SameLine();

        if (ImGui::Button("Client")) {
            _eventSink(AppEvent::StartClient);
        }

        ImGui::End();
    }
};

}
