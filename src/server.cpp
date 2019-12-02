/*
    server.cpp

    Reads from clients and pushes information
    to the aggregator.

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
#include <netdb.h>
#include <pthread.h>
#include <mutex>
#include "threadpool.h"
#include <signal.h>

using namespace std;

static mutex threadLock;
static int sockfdC;
static int sockfdS;

static void sigHandler(int s){
    cout << "Caught signal to exit" << endl;
    close(sockfdC);
    close(sockfdS);
    exit(1);
}

void messageReader(int clientSockfd)
{
    while (true) {
        char buffer[512];
        const int nR = read(clientSockfd, buffer, 512);
        if (nR == 0) {
            cout << "Connection closed" << endl;
            break;
        } else if (nR < 0) {
            cout << "Error reading from socket" << endl;
            break;
        } 
        const int nW = write(sockfdS, buffer, sizeof(buffer));
        if (nW < 0) {
            cout << "Error writing to socket." << endl;
            break;
        }
    }
    close(clientSockfd);
}

int main(int argc, char *argv[])
{
    // Get port from user
    if (argc < 4) {
        cout << "Usage: ./server <clientport> <hostname> <serverport>" << std::endl;
        return 0;
    }

    // Create a socket for client side
    sockfdC = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdC < 0) {
        cout << "Error opening socket" << endl;
        return 0;
    }

    // Create a socket for aggregator side
    sockfdS = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdS < 0) {
        cout << "Error opening socket" << endl;
        return 0;
    }

    // Validate host
    struct hostent *aggregator;
    aggregator = gethostbyname(argv[2]);
    if (aggregator == NULL) {
        cout << "Error, no such host" << endl;
        return 0;
    }

    // Bind socket to port
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    const int portNumberC = atoi(argv[1]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumberC);
    if (bind(sockfdC, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Error on binding." << endl;
        return 0;
    }

    // Connect to aggregator
    struct sockaddr_in aggregatorAddress;
    bzero((char *)&aggregatorAddress, sizeof(aggregatorAddress));
    aggregatorAddress.sin_family = AF_INET;
    bcopy((char *)aggregator->h_addr,
          (char *)&aggregatorAddress.sin_addr.s_addr,
          aggregator->h_length);
    const int portNumberS = atoi(argv[3]);
    aggregatorAddress.sin_port = htons(portNumberS);
    if (connect(sockfdS, (struct sockaddr *)&aggregatorAddress, sizeof(aggregatorAddress)) < 0) {
        cout << "Error connecting to aggregator" << endl;
        return 0;
    }

    // Setup Listener
    listen(sockfdC, 5);
    struct sockaddr_in clientAddress;
    socklen_t clientLength;
    clientLength = sizeof(clientAddress);

    // Handler for clean exit
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    int clientSockfd;
    ThreadPool pool(10);
    while (true) {
        cout << "Waiting for connections..." << endl;
        clientSockfd = accept(sockfdC,
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