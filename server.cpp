#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "include/secure_io.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: port" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);

    sockaddr_in servAddr{};
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(static_cast<std::uint16_t>(port));

    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0) {
        std::cerr << "Error establishing the server socket" << std::endl;
        return 1;
    }
    int yes = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(serverSd, reinterpret_cast<sockaddr*>(&servAddr), sizeof(servAddr)) < 0) {
        std::cerr << "Error binding socket to local address" << std::endl;
        return 1;
    }
    std::cout << "Waiting for a client to connect..." << std::endl;
    listen(serverSd, 5);

    sockaddr_in newSockAddr{};
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd = accept(serverSd, reinterpret_cast<sockaddr*>(&newSockAddr), &newSockAddrSize);
    if (newSd < 0) {
        std::cerr << "Error accepting request from client!" << std::endl;
        return 1;
    }
    std::cout << "Connected with client! (DES-encrypted channel)" << std::endl;

    struct timeval start1, end1;
    gettimeofday(&start1, nullptr);

    while (true) {
        std::cout << "Awaiting client response..." << std::endl;
        auto msg = recv_encrypted(newSd);
        if (!msg) {
            std::cout << "Connection lost or invalid data (wrong key?)" << std::endl;
            break;
        }
        if (*msg == "exit") {
            std::cout << "Client has quit the session" << std::endl;
            break;
        }
        std::cout << "Client: " << *msg << std::endl;

        std::cout << ">";
        std::string data;
        if (!std::getline(std::cin, data)) data = "exit";

        if (!send_encrypted(newSd, data)) {
            std::cerr << "Send failed" << std::endl;
            break;
        }
        if (data == "exit") break;
    }

    gettimeofday(&end1, nullptr);
    close(newSd);
    close(serverSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec) << " secs" << std::endl;
    std::cout << "Connection closed..." << std::endl;
    return 0;
}