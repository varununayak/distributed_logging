/*
    aggregator.cpp

    Handles multiple servers and stores logs
    in a .txt file. 

    @author: Varun Nayak
    @date: December 2019
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

static mutex outputFileLock;        // thread safety for output file
static int sockfd;                  // socket   
static ofstream outputfile;         // file to which we output logs

/*  currentDateTime()

    This function returns a string representing
    the current date and time in a specific 
    format.

    @return: string buf     // date and time string
*/
const string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%X", &tstruct);
    return buf;
}

/* sigHandler

    This function handles the exit signal 
    i.e. Ctrl + C from the console to perform
    a clean exit of the program.

    @param: int s           // signal id int
*/
void sigHandler(int s){
    cout << "Caught signal " << s << " to exit" << endl;
    close(sockfd);
    outputfile.close();
    exit(1);
}

/*  serverHandler
    
    Threadworker that handles each server connected to the aggregator.
    Reads from the servers and prints out the log info, writes to the file.

    @param: int clientSockfd        // client socket
*/
void serverHandler(int serverSockfd)
{
    while (true) {
        char buffer[512];
        const int n = recv(serverSockfd, buffer, 512, MSG_WAITALL);
        if (n == 0) {
            cout << "Connection closed" << endl;
            break;
        } else if (n < 0) {
            cout << "Error reading from socket" << endl;
            break;
        }
        cout << buffer;
        outputFileLock.lock();
        outputfile << buffer;
        outputFileLock.unlock();
    }
    close(serverSockfd);
}

int main(int argc, char *argv[])
{
    // Get port from user
    if (argc < 2) {
        cout << "Usage: ./aggregator <serverport>" << std::endl;
        return 0;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        return 0;
    }

    // Bind socket to port
    const int portNumber = atoi(argv[1]);
    if(!bindSocketToPort(sockfd, portNumber)) {
        return 0;
    }

    // Setup Listener
    listen(sockfd, 5);
    struct sockaddr_in serverAddress;
    socklen_t serverLength;
    serverLength = sizeof(serverAddress);

    // Handler for clean exit
    createSigHandler(sigHandler);

    // We output logs to this file
    outputfile.open("../logs/logs" + currentDateTime() + ".txt");

    int serverSockfd;
    ThreadPool pool(MAX_NUM_HANDLER_THREADS);
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
        pool.enqueue([serverSockfd] {serverHandler(serverSockfd);});
    }

    return 0;
}