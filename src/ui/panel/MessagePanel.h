#pragma once

#include "IPanel.h"

#include <imgui.h>
// #include "misc/cpp/imgui_stdlib.h"
// #include "misc/cpp/imgui_stdlib.cpp"
#include "message/NetMessage.h"

namespace otherside {

class InputMessagePanel : public IPanel {
public:
    explicit InputMessagePanel(std::shared_ptr<UiMessageFeed> feed) : _feed(feed) {}

    void render() {
        ImGui::BeginChild("InputPanel", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2), false);


        // ImGui::SetNextItemWidth(-ImGui::GetFrameHeightWithSpacing() * 2);
        ImGui::InputText("Message", _messageBuf, sizeof(_messageBuf));

        ImGui::SameLine();

        if (ImGui::Button("Send")) {
            sendMessage();
        }

        ImGui::EndChild();
    }

private:
    void sendMessage() {
        if (_messageBuf[0] == '\0') return;
        _feed->push(UiMessage{DCMessageType::MESSAGE, "Client", _messageBuf});
        _messageBuf[0] = '\0';
    }

    std::shared_ptr<UiMessageFeed> _feed;
    char _messageBuf[512] = {};
};

class MessagePanel : public IPanel {
public:
    explicit MessagePanel(std::shared_ptr<IMessageFeed> feed) : _feed(feed) {}

    void render() {
        for (auto& msg : _feed->consume()) {
            _messages.push_back(msg);
        }

        float inputHeight = ImGui::GetFrameHeightWithSpacing() * 2.5f;
        auto avail = ImGui::GetContentRegionAvail();

        ImGui::BeginChild(
            "MessagePanel",
            ImVec2(0, avail.y - inputHeight),
            true,
            ImGuiWindowFlags_HorizontalScrollbar
        );

        for (const auto& msg : _messages) {
            ImGui::TextWrapped("[%s] %s", msg.from.c_str(), msg.text.c_str());
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
