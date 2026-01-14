#pragma once
#include "IScreen.h"
#include <imgui.h>

namespace otherside {

class ClientScreen : public IScreen {
public:
    ClientScreen(EventSink sink) : IScreen(sink) {}

    void render() override {
        ImGui::Begin("Client");

        ImGui::TextColored(ImVec4(0,0.7f,1,1), "Mode: CLIENT");

        ImGui::End();
    }
};

}
