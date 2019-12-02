/*
    client.cpp

    Handles multiple threads that log
    information to a server.

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
#include <signal.h>
#include <thread>
#include <pthread.h>
#include <sstream>
#include <mutex>
#include <ctime>

using namespace std;

static mutex threadLock;
static int sockfd;


static void sigHandler(int s){
    cout << "Caught signal to exit" << endl;
    close(sockfd);
    exit(1);
}

const std::string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d | %X", &tstruct);
    return buf;
}

void kinematicsThread(string hostName)
{
    while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(1500));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        stringstream message;
        string logMessage = "<Message From Kinematics Thread>";
        // Get current time and date
        threadLock.lock();
        string dateTime = currentDateTime();
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << endl;
        string messageFromClient = message.str();
        const int n = write(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()));
        threadLock.unlock();
        if (n < 0) {
            cout << "Error writing to socket." << endl;
            break;
        }
    }
}

void dynamicsThread(string hostName)
{
    while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(1600));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        // Get current time and date
        stringstream message;
        string logMessage = "<Message From Dynamics Thread>";
        threadLock.lock();
        string dateTime = currentDateTime();
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << endl;
        string messageFromClient = message.str();
        const int n = write(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()));
        threadLock.unlock();
        if (n < 0) {
            cout << "Error writing to socket." << endl;
            break;
        }
    }
}

void controlsThread(string hostName)
{
        while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(1700));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        // Get current time and date
        stringstream message;
        string logMessage = "<Message From Controls Thread>";
        threadLock.lock();
        string dateTime = currentDateTime();
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << endl;
        string messageFromClient = message.str();
        const int n = write(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()));
        threadLock.unlock();
        if (n < 0) {
            cout << "Error writing to socket." << endl;
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    // Get hostname and port from user
    if (argc < 3) {
        cout << "Usage: ./client <hostname> <clientport>" << std::endl;
        return 0;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error while opening socket.";
    }

    // Validate host
    struct hostent *server;
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        cout << "Error, no such host" << endl;
        return 0;
    }

    // Connect to server
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serverAddress.sin_addr.s_addr,
          server->h_length);
    const int portNumber = atoi(argv[2]);
    serverAddress.sin_port = htons(portNumber);
    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Error connecting to server" << endl;
        return 0;
    }

    // Handler for clean exit
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    string hostName = string(argv[1]);
    // Create threads
    thread kinThread(kinematicsThread, hostName);
    thread dynThread(dynamicsThread, hostName);
    thread ctrlThread(controlsThread, hostName);

    kinThread.join();
    dynThread.join();
    ctrlThread.join();

    return 0;
}