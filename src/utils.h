
/*
    utils.h

    Networking and other helper functions for the distributed
    logging system. Mostly from IBM socket programming tutorials.

    @author: Varun Nayak
    @date: December 2019
 */
#pragma once

#include <iostream>
#include <unistd.h>
#include <chrono>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include <pthread.h>
#include <sstream>
#include <mutex>
#include <fstream>
#include <ctime>

#define MAX_NUM_HANDLER_THREADS 4

using namespace std;

bool bindSocketToPort(int sockfd, int portNumber)
{
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);
    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Error on binding." << endl;
        return false;
    }
    return true;
}

bool connectToServer(int sockfd, int portNumber, struct hostent* server)
{
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serverAddress.sin_addr.s_addr,
          server->h_length);
    serverAddress.sin_port = htons(portNumber);
    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Error on connecting." << endl;
        return false;
    }
    return true;
}

void createSigHandler (void (*f)(int))
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = f;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}