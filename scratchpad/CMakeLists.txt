cmake_minimum_required(VERSION 3.16)
project(PcapLearn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(readni read_net_inerface.cpp)
target_link_libraries(readni PUBLIC pcap)

add_executable(testpcap1  testpcap1.cpp)
target_link_libraries(testpcap1  PUBLIC pcap)
