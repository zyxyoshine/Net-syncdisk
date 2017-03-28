cmake_minimum_required(VERSION 2.6)
PROJECT (tmy_net)

# set(CMAKE_CURRENT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
# set(COMMON_DIR ../common)
# INCLUDE_DIRECTORIES($(COMMON_DIR))

SET(CMAKE_CXX_FLAGS  "-std=c++11")

add_library(tmy_net packets.cpp controller.cpp listener.cpp tsock.cpp sender.cpp receiver.cpp)