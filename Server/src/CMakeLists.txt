cmake_minimum_required(VERSION 2.6)
# PROJECT (TMY_TEST)

# set(CMAKE_CURRENT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
# set(COMMON_DIR ../common)
# INCLUDE_DIRECTORIES($(COMMON_DIR))
SET(CMAKE_CXX_FLAGS  "-std=c++11")
add_subdirectory(common)
add_subdirectory(server)
