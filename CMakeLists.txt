cmake_minimum_required(VERSION 3.16)
project(flow_traders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(flow_core STATIC src/StatsAggregator.cpp)
target_include_directories(flow_core PUBLIC include)

add_library(pcap_reader STATIC src/PcapDirectoryProcessor.cpp src/PcapFileReader.cpp)
target_include_directories(pcap_reader PUBLIC include)
target_link_libraries(pcap_reader PUBLIC flow_core pcap)

add_executable(feed_arbitration src/main.cpp)
target_link_libraries(feed_arbitration PRIVATE pcap_reader)

option(BUILD_UNIT_TESTS "Build unit tests" ON)
if (BUILD_UNIT_TESTS)
  enable_testing()
  add_executable(stats_tests tests/StatsAggregatorTests.cpp)
  target_link_libraries(stats_tests PRIVATE flow_core)
  add_test(NAME stats_tests COMMAND stats_tests)
endif ()
