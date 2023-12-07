cmake_minimum_required(VERSION 3.26)
project(tcms)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB SRC "${SOURCES_DIR}/*.cpp")
message("${SRC}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" ${SOURCES_DIR})
add_executable(tcms main.cpp "${SRC}")
