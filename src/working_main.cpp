#include "core/Logger.h"
#include "device/DeviceApp.h"

#include <rtc/rtc.h>
#include <thread>
#include <variant>
#include <asio.hpp>


// TCP SIGNALING
std::string readLine(asio::ip::tcp::socket& socket) {
    asio::streambuf buf;
    asio::read_until(socket, buf, "\n");
    std::istream input(&buf);
    std::string line;
    std::getline(input, line);
    return line;
}

void writeLine(asio::ip::tcp::socket& socket, const std::string& line){
    asio::write(socket, asio::buffer(line + "\n"));
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  ./otherSide host\n";
        std::cout << "  ./otherSide client <host_ip>\n";
        return 0;
    }

    rtc::InitLogger(rtc::LogLevel::Debug);

    bool isHost = std::string(argv[1]) == "host";

    asio::io_context io;

    std::shared_ptr<rtc::PeerConnection> pc;
    rtc::Configuration config;
    config.iceServers.clear();

    if (isHost) {
        std::cout << "[HOST] Starting signaling server..." << std::endl;

        asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9000));
        asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);

        std::cout << "[HOST] Client connected over TCP." << std::endl;

        pc = std::make_shared<rtc::PeerConnection>(config);


        // Create datachannel
        auto dc = pc->createDataChannel("test");

        // Callbacks
        dc->onOpen([dc](){
            std::cout << "[HOST] DataChannel open!" << std::endl;
            dc->send("Hello from HOST");
        });

        dc->onMessage([](std::variant<rtc::binary, rtc::string> msg) {
            if (std::holds_alternative<rtc::string>(msg)) {
                std::cout << "[HOST] received : " << get<rtc::string>(msg) << std::endl;
            }
        });

        // local SDP
        pc->onLocalDescription([&](rtc::Description desc) {
            std::string sdp = std::string(desc);
            writeLine(socket, sdp);
        });

        // remote ICE
        pc->onLocalCandidate([&](rtc::Candidate cand) {
            std::string ice = std::string(cand);
            writeLine(socket, ice);
        });

        // Trigger offer
        pc->setLocalDescription();

        // read remote SDP
        std::string remoteSdp = readLine(socket);
        pc->setRemoteDescription(rtc::Description(remoteSdp, "answer"));

        // read remote ICEs
        try {
            while (true) {
                std::string ice = readLine(socket);
                pc->addRemoteCandidate(rtc::Candidate(ice));
            }
        } catch (...) {}

    } else {
        std::string hostIp = argv[2];
        std::cout << "[CLIENT] Connecting to " << hostIp << "...\n";

        asio::ip::tcp::socket socket(io);
        socket.connect({asio::ip::make_address(hostIp), 9000});

        pc = std::make_shared<rtc::PeerConnection>(config);

        // datachannel callback
        pc->onDataChannel([&](std::shared_ptr<rtc::DataChannel> dc) {
            dc->onOpen([dc]() {
                std::cout << "[CLIENT] DC open!" << std::endl;
                dc->send("Hello from CLIENT");
            });

            dc->onMessage([](std::variant<rtc::binary, rtc::string> msg) {
                if (std::holds_alternative<rtc::string>(msg)) {
                    std::cout << "[CLIENT] Received : " << get<rtc::string>(msg) << std::endl;
                }
            });
        });

        // Send our answer SDP
        pc->onLocalDescription([&](rtc::Description desc) {
            writeLine(socket, std::string(desc));
        });

        // Send our ICE
        pc->onLocalCandidate([&](rtc::Candidate cand) {
            writeLine(socket, std::string(cand));
        });

        // Read remote offer SDP
        std::string offer = readLine(socket);
        pc->setRemoteDescription(rtc::Description(offer, "offer"));

        pc->setLocalDescription();


        // Receive ICEs
        try {
            while (true) {
                std::string ice = readLine(socket);
                pc->addRemoteCandidate(rtc::Candidate(ice));
            }
        } catch (...) {}

    }

    io.run();
    return 0;
}