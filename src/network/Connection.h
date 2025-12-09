#pragma once
#include <rtc/rtc.hpp>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class Connection {
    public:
    using Binary = std::vector<std::byte>;
    using OnLocalDesc = std::function<void(const std::string&)>;
    using OnLocalCandidate = std::function<void(const std::string&)>;
    using OnOpen = std::function<void()>;
    using OnMessage = std::function<void(const std::variant<rtc::binary, rtc::string>&)>;

    Connection();

    // Create offer (host) or setRemote+CreateAnswer (client)
    void createOffer();
    void setRemoteDescription(const std::string& sdp, const std::string& type);
    void createAnswer();

    // candidates
    void addRemoteCandidate(const std::string &cand);

    // send
    void sendBinary(const Binary& b);
    void sendText(const std::string& s);

    // callbacks
    void setOnLocalDescription(OnLocalDesc cb) { onLocalDescription = std::move(cb); }
    void setOnLocalCandidate(OnLocalCandidate cb) { onLocalCandidate = std::move(cb); }
    void setOnOpen(OnOpen cb) { onOpen = std::move(cb); }
    void setOnMessage(OnMessage cb) { onMessage = std::move(cb); }

    private:
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel> dc;

    OnLocalDesc onLocalDescription;
    OnLocalCandidate onLocalCandidate;
    OnOpen onOpen;
    OnMessage onMessage;
};
