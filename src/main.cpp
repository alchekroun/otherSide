#include <iostream>
#include "Signaler.h"
#include "Client.h"
#include "Server.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  ./otherSide host\n";
        std::cout << "  ./otherSide client <host_ip>\n";
        return 0;
    }
    auto type = std::string(argv[1]);
    if (type == "host") {
        auto t = std::thread([]{
            auto signalingServer = otherside::Server(8000);
            signalingServer.start();
        });
        t.join();
    } else if (type == "client") {
        auto t = std::thread([]{
            auto client = otherside::Client("127.0.0.1", 8000);
            client.start();
        });
        t.join();
    }
    return 0;
}