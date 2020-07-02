/*
    network-utils.h

    Networking and other helper functions for the distributed
    logging system. Mostly from IBM socket programming tutorials.

    @author: Varun Nayak
    @date: December 2019
 */
#pragma once

#define MAX_NUM_HANDLER_THREADS 4

bool bindSocketToPort(int sockfd, int portNumber);
bool connectToServer(int sockfd, int portNumber, struct hostent* server);
void createSigHandler(void (*f)(int));