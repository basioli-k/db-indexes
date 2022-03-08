cmake_minimum_required(VERSION 3.16)
project(Generator LANGUAGES CXX)

if(WIN32)
set(CMAKE_CXX_STANDARD 23)
else()
set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
