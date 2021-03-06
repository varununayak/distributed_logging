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
#include <netdb.h>
#include <string>
#include <sstream>
#include <mutex>

#include "thread-pool.h"
#include "network-utils.h"
#include "utils.h"

#define NUM_WORKER_THREADS 3

using namespace std;

static mutex socketLock;    // for thread safety on socket
static int sockfd;

/* sigHandler

    This function handles the exit signal 
    i.e. Ctrl + C from the console to perform
    a clean exit of the program.

    @param: int s           // signal id int
*/
void sigHandler(int s){
    cout << "Caught signal " << s << " to exit" << endl;
    close(sockfd);
    exit(1);
}

/* kinematicsThread

    This function is a dummy function to 
    simulate what would happen in an actual 
    robotics software application.
    Kinematics/Dynamics/Control/etc.

    @param: hostName        // string representing name of host
*/
void kinematicsThread(const string hostName)
{
    while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(1500));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        string logMessage = "<Message From Kinematics Thread>";
        // Get current time and date
        stringstream message;
        socketLock.lock();
        string dateTime = currentDateTime();
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << flush <<  endl;
        string messageFromClient = message.str();
        // Write to socket
        const int n = send(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()), MSG_WAITFORONE);
        socketLock.unlock();
        if (n < 0) {
            cerr << "Error writing to socket." << endl;
            continue;
        }
    }
}

/* dynamicsThread

    This function is a dummy function to 
    simulate what would happen in an actual 
    robotics software application.
    Kinematics/Dynamics/Control/etc.

    @param: hostName        // string representing name of host
*/
void dynamicsThread(const string hostName)
{
    while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(2000));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        // Get current time and date
        string logMessage = "<Message From Dynamics Thread>";
        socketLock.lock();
        string dateTime = currentDateTime();
        stringstream message;
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << flush << endl;
        string messageFromClient = message.str();
        // Write to socket
        const int n = send(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()), MSG_WAITFORONE);
        socketLock.unlock();
        if (n < 0) {
            cerr << "Error writing to socket." << endl;
            continue;
        }
    }
}

/* controlsThread

    This function is a dummy function to 
    simulate what would happen in an actual 
    robotics software application.
    Kinematics/Dynamics/Control/etc.

    @param: hostName        // string representing name of host
*/
void controlsThread(const string hostName)
{
        while (true) {
        // Wait for some seconds between each log
        this_thread::sleep_for(chrono::milliseconds(2300));
        // Get thread ID
        thread::id threadID = this_thread::get_id();
        // Get process ID
        pid_t pid = getpid();
        // Get current time and date
        string logMessage = "<Message From Controls Thread>";
        socketLock.lock();
        string dateTime = currentDateTime();
        stringstream message; 
        message << hostName <<" | " << pid << " | " << threadID << " | " << dateTime << " | DEBUG | " << logMessage << flush << endl;
        string messageFromClient = message.str();
        // Write to socket
        const int n = send(sockfd, messageFromClient.c_str(), strlen(messageFromClient.c_str()), MSG_WAITFORONE);
        socketLock.unlock();
        if (n < 0) {
            cerr << "Error writing to socket." << endl;
            continue;
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
        cerr << "Error while opening socket.";
    }

    // Validate host
    struct hostent* server;
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        cerr << "Error, no such host" << endl;
        exit(1);
    }

    // Connect to server
    const int portNumber = atoi(argv[2]);
    if(!connectToServer(sockfd, portNumber, server)){
        return 0;
    }

    // Handler for clean exit
    createSigHandler(sigHandler);

    const string hostName = string(argv[1]);
    // Create the worker threads
    ThreadPool pool(NUM_WORKER_THREADS);
    pool.enqueue([hostName] {kinematicsThread(hostName);});
    pool.enqueue([hostName] {dynamicsThread(hostName);});
    pool.enqueue([hostName] {controlsThread(hostName);});

    return 0;
}