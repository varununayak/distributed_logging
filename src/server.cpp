/*
    server.cpp

    @author: Varun Nayak
    @date: November 2019
 */
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <pthread.h>
#include <mutex>
#include "threadpool.h"
#include <signal.h>

using namespace std;

static mutex threadLock;
static int sockfd;

void sigHandler(int s){
    cout << "Caught signal to exit" << endl;
    close(sockfd);
    exit(1);
}

void messageReader(int clientSockfd)
{
    while (true) {
        char buffer[512];
        // TODO(vn): This lock may not be necessary
        threadLock.lock(); 
        const int n = read(clientSockfd, buffer, 512);
        if (n == 0) {
            cout << "Connection closed" << endl;
            break;
        } else if (n < 0) {
            cout << "Error reading from socket" << endl;
            break;
        }  
        cout << buffer; // WRITE TO AGGREGATOR SOCKET HERE
        threadLock.unlock();
    }
    close(clientSockfd);
}

int main(int argc, char *argv[])
{
    // Get port from user
    if (argc < 2) {
        cout << "Usage: ./server <port>" << std::endl;
        return 0;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        return 0;
    }

    // Bind socket to port
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    const int portNumber = atoi(argv[1]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);
    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Error on binding." << endl;
        return 0;
    }

    // Setup Listener
    listen(sockfd, 5);
    struct sockaddr_in clientAddress;
    socklen_t clientLength;
    clientLength = sizeof(clientAddress);

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    int clientSockfd;
    ThreadPool pool(4);
    while (true) {
        cout << "Waiting for connections..." << endl;
        clientSockfd = accept(sockfd,
                       (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSockfd < 0) {
            cout << "Error on accepting connection." << endl;
            continue;
        } else {
            cout << "Server got connection from " << inet_ntoa(clientAddress.sin_addr) << 
            " Port: " << ntohs(clientAddress.sin_port) << endl;
        }
        // Use threadpool to handle clients
        pool.enqueue([clientSockfd] {messageReader(clientSockfd);});
    }
    return 0;
}