//
//  client.cpp
//  netApp
//
//  Created by Jan Cakl on 03.03.2022.
//

#include "client.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 12345

#define UNKNOWN_MESS 1

void printHelp()
{
    std::cout << "Help:" << std::endl;
    std::cout << "webgrab -s .. end of daemon" << std::endl;
    std::cout << "webgrab -d url_adress .. download data" << std::endl;
}

int clientApp(messageType msg, std::string& urlAdress){

    char buff[1024];// = "";
    
    switch(msg){
        case messageType::quit:
            strcpy(buff, "s");
            std::cout << "Send quit message" << std::endl;
            break;
        case messageType::addUrl:
            strcpy(buff, ("d " + urlAdress).c_str());
            std::cout << "Add URL adress to queue" << std::endl;
            break;
        case messageType::help:
            printHelp();
            return EXIT_SUCCESS;
        default:
            std::cout << "Unknown message..exit" << std::endl;
            return UNKNOWN_MESS;
    };
    
    
    int sock_fd = -1;
    struct sockaddr_in6 server_addr;
    int ret;
     
    // Create socket for communication with server
    sock_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd < 0) {
        std::cout << "Create AF_INET6 socket failed" << std::endl;
        return EXIT_FAILURE;
    }
     
    // Connect to server running on localhost
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::1", &server_addr.sin6_addr);
    server_addr.sin6_port = htons(SERVER_PORT);
     
    // Try to do TCP handshake
    ret = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret < 0){
        std::cout << "connec() failed" << std::endl;
        close(sock_fd);
        return EXIT_FAILURE;
    }
    
    ret = write(sock_fd, &buff, strlen(buff));
    if (ret < 0) {
        std::cout << "write() failed" << std::endl;
        close(sock_fd);
        return EXIT_FAILURE;
    }
     
    // Wait for data from server
    ret = read(sock_fd, &buff, strlen(buff));
    if (ret < 0){
        std::cout << "read() failed" << std::endl;
        close(sock_fd);
        return EXIT_FAILURE;
    }
    
    std::cout << "Received <" << buff << "> from server" << std::endl;
     
    //TCP termination
    ret = close(sock_fd);
    if (ret < 0){
        std::cout << "close() failed" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
