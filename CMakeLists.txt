cmake_minimum_required(VERSION 3.26)
project(tcms)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(tcms main.cpp "${SOURCES}")
