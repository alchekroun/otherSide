#pragma once

#include "video/VideoProcessor.h"
#include "video/IVideoSource.h"
#include "network/Signaler.h"
#include "network/Connection.h"

#include <memory>
#include <string>

enum class DeviceState {
    Idle,
    WaitingForRemoteOffer,
    ReceivedRemoteOffer,
    WaitingForLocalAnswer,
    WaitingForRemoteAnswer,
    SDPComplete,
    ICEComplete,
    Connected,
    Streaming,
    Stopped
};

enum class DeviceEventType {
    RemoteOfferReceived,
    RemoteAnswerReceived,
    LocalOfferReady,        // local SDP produced (offer)
    LocalAnswerReady,       // local SDP produced (answer)
    IceCandidateGenerated,
    RemoteIceCandidateReceived,
    ConnectionEstablished,
    StartStreaming,
    StopRequested
};

struct DeviceEvent{
    DeviceEventType type;
    std::string data;
    DeviceEvent(DeviceEventType eventType) : type(eventType), data("") {}
};

enum DeviceMode {
    HOST,
    CLIENT
};



class EventQueue {
    public:
    void push(const DeviceEvent& ev) {
        {
            std::lock_guard<std::mutex> lg(mtx);
            queue.push(ev);
        }
        cv.notify_one();
    }

    DeviceEvent waitPop() {
        std::unique_lock<std::mutex> l(mtx);
        cv.wait(l, [this] { return !queue.empty(); });
        auto ev = queue.front();
        queue.pop();
        return ev;
    }

    void notifyAll() {cv.notify_all();}

    private:
    std::queue<DeviceEvent> queue;
    std::mutex mtx;
    std::condition_variable cv;
};

class Device {
    public:
    // mode = "host" or "client"
    Device(const std::string &mode,
           const std::string &peerIp,
           uint16_t port,
           std::unique_ptr<IVideoSource> source);

    ~Device();

    void run();

    void pushEvent(DeviceEvent ev);
    void setState(DeviceState s);
    DeviceState getState() const;
    void waitForState(DeviceState expected);

private:
    void handleEvent(DeviceEvent ev);
    void handleSignalerLine(const std::string &line);
    void processFrameLoop();
    void sendFrame(const VideoFrame &frame);

    std::string mode;
    std::string peerIp;
    uint16_t port;

    Signaler signaler;
    Connection conn;
    VideoProcessor processor;
    std::unique_ptr<IVideoSource> source;

    EventQueue eventQueue;
    std::atomic<DeviceState> state{DeviceState::Idle};
    mutable std::mutex stateMutex;
    std::condition_variable stateCV;

    std::atomic_bool running{false};
};
