#include "Device.h"

Device::Device(const std::string &mode_,
               const std::string &peerIp_,
               uint16_t port_,
               std::unique_ptr<IVideoSource> source_)
: mode(mode_), peerIp(peerIp_), port(port_), source(std::move(source_)) {

    // wire signaling clb
    signaler.setOnLine([this](const std::string& line) {
        this->handleSignalerLine(line);
    });

    // wire rtc clbs
    conn.setOnLocalDescription([this](const std::string& sdp){
        std::cout << "[DEVICE] Local sdp ready, sending via signaling\n";
        signaler.sendLine(sdp);
    });

    conn.setOnLocalCandidate([this](const std::string& cand) {
        std::cout << "[DEVICE] got a local candidate : CAND:" << cand << std::endl;
        signaler.sendLine(std::string("CAND:" + cand));
    });

    conn.setOnOpen([this]() {
        std::cout << "[DEVICE] DataChannel OPEN\n";
    });

    conn.setOnMessage([this](const std::variant<rtc::binary, rtc::string>& m) {
        if (std::holds_alternative<rtc::string>(m)) {
            std::cout << "[DEVICE] remote text: " << std::get<rtc::string>(m) << std::endl;
        } else {
            std::cout << "[DEVICE] remote binary frame received (not processed in example)\n";
        }
    });
}

Device::~Device() {
    pushEvent(DeviceEvent(DeviceEventType::StopRequested));
    running.store(false);
    signaler.stop();
}

void Device::pushEvent(DeviceEvent ev) {
    std::lock_guard<std::mutex> lg(stateMutex);
    eventQueue.push(ev);
}

void Device::handleSignalerLine(const std::string &line) {
    // Distinguish candidates
    if (line.rfind("CAND:", 0) == 0) {
        std::string cand = line.substr(5);
        conn.addRemoteCandidate(cand);
        return;
    }

    // otherwise treat as SDP (offer / answer)
    // we must determine if incoming SDP is offer or answer; libdatachannel uses "offer"/"answer" text in Description
    // For simplicity, check if contains "a=mid:" or "m=" and guess: if we're host and we received an answer, apply as answer
    // Simpler: we assume host will first send offer; client will send back answer
    if (mode == "host") {
        // host receives answer from client
        conn.setRemoteDescription(line, "answer");
        std::cout << "[Device] Host received remote answer\n";
    } else {
        // client receives offer from host
        conn.setRemoteDescription(line, "offer");
        std::cout << "[Device] Client received remote offer; creating answer\n";
        conn.createAnswer();
    }
}


void Device::run() {
    running.store(true);
    if (mode == "host") {
        if (!signaler.startHost(port)) {
            std::cerr << "Failed to start signaling host\n";
            return;
        }
        std::cout << "[DEVICE] Host creating offer\n";
        setState(DeviceState::WaitingForRemoteAnswer);
        conn.createOffer();
    } else {
        if (!signaler.startClient(peerIp, port)) {
            std::cerr << "Failed to start signaling client\n";
            return;
        }
        setState(DeviceState::WaitingForRemoteOffer);
    }

    std::thread frameThread([this]() {this->processFrameLoop(); });

    while(running) {
        handleEvent(eventQueue.waitPop());
    }
}

void Device::setState(DeviceState s) {
    std::lock_guard<std::mutex> lg(stateMutex);
    state = s;
}

DeviceState Device::getState() const {
    std::lock_guard<std::mutex> lg(stateMutex);
    return state;
}

void Device::waitForState(DeviceState expected) {
    std::unique_lock<std::mutex> ul(stateMutex);
    stateCV.wait(ul, [this, expected]{ return state == expected; });
}

void Device::handleEvent(DeviceEvent de) {
    switch (de.type)
    {
    case DeviceEventType::RemoteOfferReceived:
        setState(DeviceState::ReceivedRemoteOffer);
        break;
    case DeviceEventType::RemoteAnswerReceived:
        break;
    case DeviceEventType::LocalOfferReady:
        setState(DeviceState::SDPComplete);
        break;
    case DeviceEventType::LocalAnswerReady:
        setState(DeviceState::SDPComplete);
        break;
    case DeviceEventType::IceCandidateGenerated:
        setState(DeviceState::ICEComplete);
        break;
    case DeviceEventType::RemoteIceCandidateReceived:
        break;
    case DeviceEventType::ConnectionEstablished:
        setState(DeviceState::Connected);
        break;
    case DeviceEventType::StartStreaming:
        setState(DeviceState::Streaming);
        break;
    case DeviceEventType::StopRequested:
        setState(DeviceState::Stopped);
        break;
    default:
        break;
    }
}

void Device::processFrameLoop() {
    waitForState(DeviceState::Streaming);
    VideoFrame frame;
    while(running && getState() != DeviceState::Stopped) {
        if (!source->nextFrame(frame)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        VideoFrame processed;
        if (!processor.process(frame, processed)) continue;
        sendFrame(processed); // NE PAS STEP BY STEP HERE
    }
}

void Device::sendFrame(const VideoFrame& frame) {
    // serialize
    std::vector<std::byte> buf;
    buf.reserve(8 + frame.data.size());
    uint32_t w = frame.width;
    uint32_t h = frame.height;
    // big endian
    auto push32 = [&buf](uint32_t v){
        buf.push_back(std::byte((v >> 24) & 0xFF));
        buf.push_back(std::byte((v >> 16) & 0xFF));
        buf.push_back(std::byte((v >> 8) & 0xFF));
        buf.push_back(std::byte((v) & 0xFF));
    };
    push32(w);
    push32(h);
    for (auto &b : frame.data) buf.push_back(std::byte(b));
    conn.sendBinary(buf);
}