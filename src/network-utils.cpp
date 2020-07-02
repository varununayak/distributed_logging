
/*
    network-utls.cpp

    Networking and other helper functions for the distributed
    logging system. Mostly from IBM socket programming tutorials.

    @author: Varun Nayak
    @date: December 2019
 */
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#include "network-utils.h"

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