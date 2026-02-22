#include "MessagePanel.h"

namespace otherside
{

void DisplayMesssageFeed::pushMessage(const UiMessage &msg)
{
    _txFeed->push(msg);
    _messages.push_back(msg);
}

std::vector<UiMessage> DisplayMesssageFeed::getMessages()
{
    update();
    return _messages;
}

void DisplayMesssageFeed::update()
{
    for (auto &msg : _rxFeed->consume())
    {
        _messages.push_back(msg);
    }
}

void InputMessagePanel::render()
{
    ImGui::BeginChild("InputPanel", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2), 0);

    // ImGui::SetNextItemWidth(-ImGui::GetFrameHeightWithSpacing() * 2);
    ImGui::InputText("Message", _messageBuf, sizeof(_messageBuf));

    ImGui::SameLine();

    if (ImGui::Button("Send"))
    {
        sendMessage();
    }

    ImGui::EndChild();
}

void InputMessagePanel::sendMessage()
{
    if (_messageBuf[0] == '\0')
    {
        return;
    }
    _feed->pushMessage(UiMessage{DCMessageType::MESSAGE, _source, utils::nowMs(), 0, _messageBuf});
    _messageBuf[0] = '\0';
}

void DisplayedMessagePanel::render()
{
    float inputHeight = ImGui::GetFrameHeightWithSpacing() * 2.5f;
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("DisplayedMessagePanel", ImVec2(0, avail.y - inputHeight), 0,
                      ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto &msg : _feed->getMessages())
    {
        if (msg.from == _source)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0.6f, 0.6f, 1));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0, 1));
        }
        ImGui::TextWrapped("[%s][%s] %s", formatTime(msg.createdTs).c_str(),
                           peerIdToString[msg.from].c_str(), msg.text.c_str());
        ImGui::PopStyleColor();
    }

    // Auto-scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

std::string DisplayedMessagePanel::formatTime(utils::TimestampMs ts)
{
    std::time_t t = static_cast<std::time_t>(ts) / 1000;
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[9];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

void MessagePanel::render()
{
    ImGui::BeginChild("MessagePanel", ImVec2(0, 0));
    _displayedMessagePanel.render();
    ImGui::Separator();
    _inputMessagePanel.render();
    ImGui::EndChild();
}

} // namespace otherside
