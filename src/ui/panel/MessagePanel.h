#pragma once
#include <imgui.h>
#include "message/NetMessage.h"

namespace otherside {

class MessagePanel {
public:
    explicit MessagePanel(std::shared_ptr<IMessageFeed> feed) : _feed(feed) {}

    void render() {
        ImGui::BeginChild(
            "MessagePanel",
            ImVec2(0, 0),
            true,
            ImGuiWindowFlags_AlwaysVerticalScrollbar
        );

        for (auto& msg : _feed->consume()) {
            _messages.push_back(msg);
        }

        for (const auto& msg : _messages) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.f, 1.f), "[%s] %s", msg.from.c_str(), msg.text.c_str());
        }

        // Auto-scroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
    }

private:
    std::shared_ptr<IMessageFeed> _feed;
    std::vector<UiMessage> _messages;
};

}
