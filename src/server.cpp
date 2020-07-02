/*
    server.cpp

    Reads from clients and pushes information
    to the aggregator.

    @author: Varun Nayak
    @date: November 2019
 */
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

#include "thread-pool.h"
#include "network-utils.h"

using namespace std;

static mutex socketLock;    // thread safety on socket
static int sockfdC;         // client side socket
static int sockfdA;         // aggregator side socket

/* sigHandler

    This function handles the exit signal 
    i.e. Ctrl + C from the console to perform
    a clean exit of the program.

    @param: int s           // signal id int
*/
void sigHandler(int s){
    cout << "Caught signal " << s << " to exit" << endl;
    close(sockfdC);
    close(sockfdA);
    exit(1);
}

/*  clientHandler
    
    Threadworker that handles each client connected to the server.
    Reads from the client and writes to the aggregators.

    @param: int clientSockfd        // client socket
*/
void clientHandler(int clientSockfd)
{
    while (true) {
        char buffer[512];
        const int nR = recv(clientSockfd, buffer, 512, MSG_WAITALL);
        if (nR == 0) {
            cout << "Connection closed" << endl;
            break;
        } else if (nR < 0) {
            cout << "Error reading from socket" << endl;
            break;
        } 
        socketLock.lock();
        const int nW = send(sockfdA, buffer, sizeof(buffer), MSG_WAITALL);
        socketLock.unlock();
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
        cerr << "Error opening socket" << endl;
        return 0;
    }

    // Create a socket for aggregator side
    sockfdA = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdA < 0) {
        cerr << "Error opening socket" << endl;
        return 0;
    }

    // Validate host
    struct hostent *aggregator;
    aggregator = gethostbyname(argv[2]);
    if (aggregator == NULL) {
        cerr << "Error, no such host" << endl;
        return 0;
    }

    // Bind socket to port on client side
    const int portNumberC = atoi(argv[1]);
    if(!bindSocketToPort(sockfdC, portNumberC)) {
        return 0;
    }
    
    // Connect to aggregator
    const int portNumberS = atoi(argv[3]);
    if(!connectToServer(sockfdA, portNumberS, aggregator)){
        return 0;
    }

    // Setup Listener
    listen(sockfdC, 5);
    struct sockaddr_in clientAddress;
    socklen_t clientLength;
    clientLength = sizeof(clientAddress);

    // Handler for clean exit
    createSigHandler(sigHandler);

    int clientSockfd;
    ThreadPool pool(MAX_NUM_HANDLER_THREADS);
    while (true) {
        cout << "Waiting for connections..." << endl;
        clientSockfd = accept(sockfdC,
                       (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSockfd < 0) {
            cerr << "Error on accepting connection." << endl;
            continue;
        } else {
            cout << "Server got connection from " << inet_ntoa(clientAddress.sin_addr) << 
            " Port: " << ntohs(clientAddress.sin_port) << endl;
        }
        // Use threadpool to handle clients
        pool.enqueue([clientSockfd] {clientHandler(clientSockfd);});
    }

    return 0;
}