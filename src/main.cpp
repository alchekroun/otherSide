#include "device/Device.h"
#include "video/FakeVideoSource.h"
#include <memory>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage:\n  otherSide host <port>\n  otherSide client <host_ip> <port>\n";
        return 0;
    }

    std::string mode = argv[1];
    uint16_t port = 9000;
    std::string hostIp;
    if (mode == "host") {
        port = static_cast<uint16_t>(std::stoi(argv[2]));
    } else {
        if (argc < 4) {
            std::cerr << "client requires host_ip and port\n";
            return 1;
        }
        hostIp = argv[2];
        port = static_cast<uint16_t>(std::stoi(argv[3]));
    }

    auto source = std::make_unique<FakeVideoSource>(160, 120, 5);
    Device device(mode, hostIp, port, std::move(source));
    device.run();

    return 0;
}

