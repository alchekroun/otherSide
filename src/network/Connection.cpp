#include "Connection.h"
#include <iostream>

Connection::Connection() {
    rtc::InitLogger(rtc::LogLevel::Debug);

    rtc::Configuration config;
    config.iceServers.clear();

    pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onLocalDescription([this](rtc::Description d) {
        if (onLocalDescription) onLocalDescription(std::string(d));
    });

    pc->onLocalCandidate([this](rtc::Candidate c) {
        if (onLocalCandidate) onLocalCandidate(std::string(c));
    });

    pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
        dc = channel;
        dc->onOpen([this]() {
            if (onOpen) onOpen();
        });

        dc->onMessage([this](std::variant<rtc::binary, rtc::string> m) {
            if (onMessage) onMessage(m);
        });
    });
}

void Connection::createOffer() {
    dc = pc->createDataChannel("video");
    dc->onOpen([this]() {if (onOpen) onOpen(); });
    dc->onMessage([this](std::variant<rtc::binary, rtc::string> m){ if (onMessage) onMessage(m); });

    pc->setLocalDescription(); // triggers onLocalDescription
}

void Connection::createAnswer() {
    pc->setLocalDescription();
}

void Connection::setRemoteDescription(const std::string& sdp, const std::string& type) {
    pc->setRemoteDescription(rtc::Description(sdp, type));
}

void Connection::addRemoteCandidate(const std::string& cand) {
    pc->addRemoteCandidate(rtc::Candidate(cand));
}

void Connection::sendBinary(const Binary& b) {
    if (dc) {
        rtc::binary bin;
        bin.reserve(b.size());
        for (const auto& bb: b) {
            bin.push_back(bb);
        }
        dc->send(bin);
    }
}

void Connection::sendText(const std::string& s) {
    if (dc) dc->send(s);
}