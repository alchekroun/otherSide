#pragma once

#include "IPanel.h"

#include <imgui.h>
// #include "misc/cpp/imgui_stdlib.h"
// #include "misc/cpp/imgui_stdlib.cpp"
#include "message/NetMessageFeed.h"
#include "utils.h"

namespace otherside
{

class DisplayMesssageFeed
{
public:
    DisplayMesssageFeed(const std::shared_ptr<IMessageFeed> &rxFeed_,
                        const std::shared_ptr<UiMessageFeed> &txFeed_)
        : _rxFeed(rxFeed_), _txFeed(txFeed_)
    {
    }

    void pushMessage(const UiMessage &msg);

    std::vector<UiMessage> getMessages();

private:
    void update();

    std::shared_ptr<IMessageFeed> _rxFeed;
    std::shared_ptr<UiMessageFeed> _txFeed;
    std::vector<UiMessage> _messages;
};

class InputMessagePanel : public IPanel
{
public:
    explicit InputMessagePanel(PeerId source, const std::shared_ptr<DisplayMesssageFeed> &feed)
        : _feed(feed), _source(source)
    {
    }

    void render() override;

private:
    void sendMessage();

    PeerId _source;
    std::shared_ptr<DisplayMesssageFeed> _feed;
    char _messageBuf[512] = {};
};

class DisplayedMessagePanel : public IPanel
{
public:
    explicit DisplayedMessagePanel(PeerId source, const std::shared_ptr<DisplayMesssageFeed> &feed)
        : _feed(feed), _source(source)
    {
    }

    void render() override;

private:
    static std::string formatTime(utils::TimestampMs ts);

    std::unordered_map<PeerId, std::string> peerIdToString = {{PeerId::HOST, "Host"},
                                                              {PeerId::CLIENT, "Client"}};
    PeerId _source;
    std::shared_ptr<DisplayMesssageFeed> _feed;
};

class MessagePanel : public IPanel
{
public:
    MessagePanel(PeerId source, const std::shared_ptr<IMessageFeed> &rxFeed_,
                 const std::shared_ptr<UiMessageFeed> &txFeed_)
        : _displayedMessageFeed(std::make_shared<DisplayMesssageFeed>(rxFeed_, txFeed_)),
          _displayedMessagePanel(source, _displayedMessageFeed),
          _inputMessagePanel(source, _displayedMessageFeed) {};

    void render() override;

private:
    std::shared_ptr<DisplayMesssageFeed> _displayedMessageFeed;
    DisplayedMessagePanel _displayedMessagePanel;
    InputMessagePanel _inputMessagePanel;
};

} // namespace otherside
