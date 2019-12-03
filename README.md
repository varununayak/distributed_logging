# distributed_logging

Distributed logging system for Dexterity Coding Challenge.

### Client
Keeping with the theme of robotics software, we have three threads (nominally three, there can be more) that represent thread workers that perform "kinematics", "dynamics" and "control" for a particular robot. Each of these threads has a particular log message that it writes to the server it is connected to. The format of the log messsage is:

host name | pid | thread id | date and time | log level |  log message

The defaut log level is DEBUG.

### Server

The server reads the log messages from clients (uses thread pooling to handle various clients) and writes the messages to the
aggreator (another socket connection).

### Aggregator

The aggregator reads the log messages from the server and outputs the messages to the console as well as to a time-stamped .txt file, stored in the logs/ folder.

### Prerequisites

You will need:

 * A modern C/C++ compiler (eg: gcc)
 * CMake 3.1+ installed (on a Mac, run `brew install cmake`)
 
Uses threadpool library from https://github.com/progschj/ThreadPool

### Building The Project
```
mkdir build && cd build
cmake .. && make
```
## Example Usage

In a terminal:
```
./aggregator 12347              # to run a aggregator
./server 12346 localhost 12345  # to run a server
./client localhost 12345        # to run a client
```
Note that the port numbers for individual connections must match.

### Known Bug

When multiple clients connect to a server, the log messages are sometimes not read from the buffer correctly.
