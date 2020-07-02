cmake_minimum_required(VERSION 3.1)

project(DistributedLogging C CXX) 

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14 -pthread")
set(source_dir "${PROJECT_SOURCE_DIR}/src/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


include_directories("include/")
add_executable(server src/server.cpp)
add_executable(client src/client.cpp)
add_executable(aggregator src/aggregator.cpp)
