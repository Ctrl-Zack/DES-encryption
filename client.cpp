#include <arpa/inet.h>
#include <netdb.h>
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
    if (argc != 3) {
        std::cerr << "Usage: ip_address port" << std::endl;
        return 1;
    }
    const char* serverIp = argv[1];
    int port = std::atoi(argv[2]);

    struct hostent* host = gethostbyname(serverIp);
    if (!host) {
        std::cerr << "Cannot resolve host" << std::endl;
        return 1;
    }

    sockaddr_in sendSockAddr{};
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr = *reinterpret_cast<in_addr*>(host->h_addr_list[0]);
    sendSockAddr.sin_port = htons(static_cast<std::uint16_t>(port));

    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(clientSd, reinterpret_cast<sockaddr*>(&sendSockAddr), sizeof(sendSockAddr)) < 0) {
        std::cerr << "Error connecting to socket!" << std::endl;
        return 1;
    }
    std::cout << "Connected to the server! (DES-encrypted channel)" << std::endl;

    struct timeval start1, end1;
    gettimeofday(&start1, nullptr);

    while (true) {
        std::cout << ">";
        std::string data;
        if (!std::getline(std::cin, data)) data = "exit";

        if (!send_encrypted(clientSd, data)) {
            std::cerr << "Send failed" << std::endl;
            break;
        }
        if (data == "exit") break;

        std::cout << "Awaiting server response..." << std::endl;
        auto reply = recv_encrypted(clientSd);
        if (!reply) {
            std::cout << "Connection lost or invalid data (wrong key?)" << std::endl;
            break;
        }
        if (*reply == "exit") {
            std::cout << "Server has quit the session" << std::endl;
            break;
        }
        std::cout << "Server: " << *reply << std::endl;
    }

    gettimeofday(&end1, nullptr);
    close(clientSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec) << " secs" << std::endl;
    std::cout << "Connection closed" << std::endl;
    return 0;
}