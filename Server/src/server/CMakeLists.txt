cmake_minimum_required(VERSION 2.6)
PROJECT (tmy_server)

SET(CMAKE_CXX_FLAGS "-I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lssl -lcrypto -ldl -std=c++11 -lssl")

add_library(tmy_server_lib logger.cpp)
add_executable(tmy_server server.cpp logger.cpp)