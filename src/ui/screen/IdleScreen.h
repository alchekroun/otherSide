#pragma once
#include "IScreen.h"
#include <imgui.h>

namespace otherside
{

class IdleScreen : public IScreen
{
  public:
    IdleScreen(EventSink sink) : IScreen(std::move(sink))
    {
    }

    void render() override
    {
        // ImGui::Begin("Client");

        // ImGui::TextColored(ImVec4(0,1,0,1), "Waiting");

        // ImGui::End();
    }
};

} // namespace otherside
