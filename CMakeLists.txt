cmake_minimum_required(VERSION 3.1)

project(DistributedLogging C CXX) 

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14 -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(UTILS "src/network-utils.cpp" "src/utils.cpp")

include_directories("include/")
add_executable(server src/server.cpp ${UTILS})
add_executable(client src/client.cpp  ${UTILS})
add_executable(aggregator src/aggregator.cpp  ${UTILS})
