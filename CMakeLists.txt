cmake_minimum_required(VERSION 3.16)
project(assignment1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(assignment1
        interrupt.h
        main.cpp
        thread.h
        waitqueue.h
        task.h
        readfile.h
        producer.h
        consumer.h
        main_new.cpp)
