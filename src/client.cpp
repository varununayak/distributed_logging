/*
    client.cpp

    Handles multiple threads that log
    information to a server.

    @author: Varun Nayak
    @date: November 2019
 */

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <thread>
#include <pthread.h>
#include <sstream>
#include <mutex>
#include <ctime>

using namespace std;

const std::string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d | %X", &tstruct);
    return buf;
}

void kinematicsThread(int sockfd, string hostName)
{
    while (true) {
        // Wait for 2 seconds between each log
        sleep(2);
        // Get current time and date
        string dateTime = currentDateTime();
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        // Stringstream makes cout thread safe
        stringstream message;
        string logMessage = "<Message From Kinematics Thread>";
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << endl;
        string messageFromClient = message.str();
        // TODO(vn): Make this thread safe
        const int n = write(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()));
        if (n < 0) {
            stringstream errorMessage;
            errorMessage << "Error writing to socket." << endl;
            cout << errorMessage.str();
            break;
        }
    }
}

void dynamicsThread(int sockfd)
{
}

void controlsThread(int sockfd)
{

}

int main(int argc, char *argv[])
{
    // Get hostname and port from user
    if (argc < 3) {
        cout << "Usage: ./client <hostname> <port>" << std::endl;
        return 0;
    }

    // Create socket
    int sockfd, portNumber;
    portNumber = atoi(argv[2]);
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
    serverAddress.sin_port = htons(portNumber);
    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Error connecting to server" << endl;
        return 0;
    }

    // Create threads
    string hostName = string(argv[1]);
    thread kinThread(kinematicsThread, sockfd, hostName);
    thread dynThread(dynamicsThread, sockfd);
    thread ctrlThread(controlsThread, sockfd);

    kinThread.join();
    dynThread.join();
    ctrlThread.join();

    close(sockfd);
    return 0;
}
