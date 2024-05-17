cmake_minimum_required(VERSION 3.10)

project(JetsonMiniFan)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

message(STATUS "CMake version ${CMAKE_VERSION}")
message(STATUS "CMake build type ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
else()
	set(CMAKE_BUILD_TYPE "Release")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

add_executable(jetson_mini_fan jetson_mini_fan.c)
