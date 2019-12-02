/*
    aggregator.cpp

    Handles multiple servers and stores logs
    in a .txt file. 

    @author: Varun Nayak
    @date: December 2019
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
#include <fstream>

using namespace std;

static mutex threadLock;
static int sockfd;
static ofstream outputfile;

static void sigHandler(int s){
    cout << "Caught signal to exit" << endl;
    close(sockfd);
    outputfile.close();
    exit(1);
}

const std::string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%X", &tstruct);
    return buf;
}

void messageReader(int aggregatorSockfd)
{
    while (true) {
        char buffer[512];
        // TODO(vn): This lock may not be necessary
        threadLock.lock(); 
        const int n = read(aggregatorSockfd, buffer, 512);
        if (n == 0) {
            cout << "Connection closed" << endl;
            break;
        } else if (n < 0) {
            cout << "Error reading from socket" << endl;
            break;
        }  
        cout << buffer;
        outputfile << buffer;
        threadLock.unlock();
    }
    close(aggregatorSockfd);
}

int main(int argc, char *argv[])
{
    // Get port from user
    if (argc < 2) {
        cout << "Usage: ./aggregator <port>" << std::endl;
        return 0;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        return 0;
    }

    // Bind socket to port
    struct sockaddr_in aggregatorAddress;
    bzero((char *)&aggregatorAddress, sizeof(aggregatorAddress));
    const int portNumber = atoi(argv[1]);
    aggregatorAddress.sin_family = AF_INET;
    aggregatorAddress.sin_addr.s_addr = INADDR_ANY;
    aggregatorAddress.sin_port = htons(portNumber);
    if (bind(sockfd, (struct sockaddr *)&aggregatorAddress, sizeof(aggregatorAddress)) < 0) {
        cout << "Error on binding." << endl;
        return 0;
    }

    // Setup Listener
    listen(sockfd, 5);
    struct sockaddr_in serverAddress;
    socklen_t serverLength;
    serverLength = sizeof(serverAddress);

    // Handler for clean exit
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    outputfile.open("../logs/logs" + currentDateTime() + ".txt");

    int serverSockfd;
    ThreadPool pool(4);
    while (true) {
        cout << "Waiting for connections..." << endl;
        serverSockfd = accept(sockfd,
                       (struct sockaddr *)&serverAddress, &serverLength);
        if (serverSockfd < 0) {
            cout << "Error on accepting connection." << endl;
            continue;
        } else {
            cout << "aggregator got connection from " << inet_ntoa(serverAddress.sin_addr) << 
            " Port: " << ntohs(serverAddress.sin_port) << endl;
        }
        // Use threadpool to handle servers
        pool.enqueue([serverSockfd] {messageReader(serverSockfd);});
    }

    return 0;
}