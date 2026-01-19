// ui/screens/RoleSelectionScreen.h
#pragma once
#include "IScreen.h"
#include <imgui.h>

namespace otherside
{

class RoleSelectionScreen : public IScreen
{
  public:
    RoleSelectionScreen(EventSink sink) : IScreen(std::move(sink))
    {
    }

    void render() override
    {
        ImGui::Begin("OtherSide");

        ImGui::Text("Choose your role:");

        if (ImGui::Button("Host"))
        {
            _eventSink(AppEvent::START_HOST);
        }

        ImGui::SameLine();

        if (ImGui::Button("Client"))
        {
            _eventSink(AppEvent::START_CLIENT);
        }

        ImGui::End();
    }
};

} // namespace otherside
