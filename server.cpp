#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: port" << std::endl;
        exit(0);
    }

    int port = atoi(argv[1]);

    char msg[1500];
     
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
 
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0) {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }

    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0) {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
    std::cout << "Waiting for a client to connect..." << std::endl;

    listen(serverSd, 5);

    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0) {
        std::cerr << "Error accepting request from client!" << std::endl;
        exit(1);
    }
    std::cout << "Connected with client!" << std::endl;

    struct timeval start1, end1;
    gettimeofday(&start1, NULL);

    int bytesRead, bytesWritten = 0;
    while(1) {
        std::cout << "Awaiting client response..." << std::endl;
        memset(&msg, 0, sizeof(msg));
        bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, "exit")) {
            std::cout << "Client has quit the session" << std::endl;
            break;
        }

        std::cout << "Client: " << msg << std::endl;
        std::cout << ">";
        std::string data;
        getline(std::cin, data);
        memset(&msg, 0, sizeof(msg));
        strcpy(msg, data.c_str());
        if(data == "exit") {
            send(newSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
    }

    gettimeofday(&end1, NULL);
    close(newSd);
    close(serverSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << std::endl;
    std::cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec) << " secs" << std::endl;
    std::cout << "Connection closed..." << std::endl;

    return 0;   
}