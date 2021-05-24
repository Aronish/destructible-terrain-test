cmake_minimum_required(VERSION 3.20.2)

project(engineering_game)

if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "/EHsc /Zi /W4")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
    set(CMAKE_CXX_FLAGS "/EHsc /O2 /W4")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(engineering_game src/main.cpp)

target_include_directories(engineering_game PRIVATE include/)