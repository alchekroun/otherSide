#pragma once

#include "IPanel.h"

#include <imgui.h>
// #include "misc/cpp/imgui_stdlib.h"
// #include "misc/cpp/imgui_stdlib.cpp"
#include "message/NetMessageFeed.h"

namespace otherside
{

class DisplayMesssageFeed
{
  public:
    DisplayMesssageFeed(std::shared_ptr<IMessageFeed> rxFeed_, std::shared_ptr<UiMessageFeed> txFeed_)
        : _rxFeed(rxFeed_), _txFeed(txFeed_)
    {
    }

    void pushMessage(UiMessage msg)
    {
        _txFeed->push(msg);
        _messages.push_back(msg);
    }

    std::vector<UiMessage> getMessages()
    {
        update();
        return _messages;
    }

  private:
    void update()
    {
        for (auto &msg : _rxFeed->consume())
        {
            _messages.push_back(msg);
        }
    }

    std::shared_ptr<IMessageFeed> _rxFeed;
    std::shared_ptr<UiMessageFeed> _txFeed;
    std::vector<UiMessage> _messages;
};

class InputMessagePanel : public IPanel
{
  public:
    explicit InputMessagePanel(PeerId source, std::shared_ptr<DisplayMesssageFeed> feed) : _feed(feed), _source(source)
    {
    }

    void render() override
    {
        ImGui::BeginChild("InputPanel", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2), false);

        // ImGui::SetNextItemWidth(-ImGui::GetFrameHeightWithSpacing() * 2);
        ImGui::InputText("Message", _messageBuf, sizeof(_messageBuf));

        ImGui::SameLine();

        if (ImGui::Button("Send"))
        {
            sendMessage();
        }

        ImGui::EndChild();
    }

  private:
    void sendMessage()
    {
        if (_messageBuf[0] == '\0')
            return;
        _feed->pushMessage(UiMessage{DCMessageType::MESSAGE, _source, nowMs(), 0, _messageBuf});
        _messageBuf[0] = '\0';
    }

    PeerId _source;
    std::shared_ptr<DisplayMesssageFeed> _feed;
    char _messageBuf[512] = {};
};

class DisplayedMessagePanel : public IPanel
{
  public:
    explicit DisplayedMessagePanel(PeerId source, std::shared_ptr<DisplayMesssageFeed> feed)
        : _feed(feed), _source(source)
    {
    }

    void render() override
    {
        float inputHeight = ImGui::GetFrameHeightWithSpacing() * 2.5f;
        auto avail = ImGui::GetContentRegionAvail();

        ImGui::BeginChild("DisplayedMessagePanel", ImVec2(0, avail.y - inputHeight), true,
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
            ImGui::TextWrapped("[%s][%s] %s", formatTime(msg.createdTs).c_str(), peerIdToString[msg.from].c_str(),
                               msg.text.c_str());
            ImGui::PopStyleColor();
        }

        // Auto-scroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
    }

  private:
    static std::string formatTime(TimestampMs ts)
    {
        std::time_t t = ts / 1000;
        std::tm tm{};
        localtime_r(&t, &tm);
        char buf[9];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
        return buf;
    }

    std::unordered_map<PeerId, std::string> peerIdToString = {{PeerId::HOST, "Host"}, {PeerId::CLIENT, "Client"}};
    PeerId _source;
    std::shared_ptr<DisplayMesssageFeed> _feed;
};

class MessagePanel : public IPanel
{
  public:
    MessagePanel(PeerId source, std::shared_ptr<IMessageFeed> rxFeed_, std::shared_ptr<UiMessageFeed> txFeed_)
        : _displayedMessageFeed(std::make_shared<DisplayMesssageFeed>(rxFeed_, txFeed_)),
          _displayedMessagePanel(source, _displayedMessageFeed), _inputMessagePanel(source, _displayedMessageFeed) {};

    void render() override
    {
        ImGui::BeginChild("MessagePanel", ImVec2(0, 0));
        _displayedMessagePanel.render();
        ImGui::Separator();
        _inputMessagePanel.render();
        ImGui::EndChild();
    }

  private:
    std::shared_ptr<DisplayMesssageFeed> _displayedMessageFeed;
    DisplayedMessagePanel _displayedMessagePanel;
    InputMessagePanel _inputMessagePanel;
};

} // namespace otherside
