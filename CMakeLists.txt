cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB srcs src/*.cpp)
file(GLOB hdrs include/*.h)

set(SOURCE_FILES ${srcs};${hdrs})

include_directories(include)

add_executable(VOSIMLib ${SOURCE_FILES};main.cpp)