/*
    utils.cpp

    Other helper functions for the distributed
    logging system.

    @author: Varun Nayak
    @date: December 2019
 */
#include <signal.h>

#include "utils.h"

using namespace std;

const string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d | %X", &tstruct);
    return buf;
}

void createSigHandler(void (*f)(int))
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = f;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}