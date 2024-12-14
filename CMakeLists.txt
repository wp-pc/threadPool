cmake_minimum_required(VERSION 3.8)

project(threadPool)


add_executable(threadPool 
    threadPool.h threadPool.cpp
    ./example/main.cpp
)