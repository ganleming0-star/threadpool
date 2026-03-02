cmake_minimum_required(VERSION 4.0)
project(threadPool)

set(CMAKE_CXX_STANDARD 20)

add_executable(threadPool main.cpp
        threadpool.h
        threadpool.cpp
)
