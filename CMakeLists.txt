cmake_minimum_required(VERSION 3.5)
project(ws CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/v*/*.cpp")
add_executable(${PROJECT_NAME} main.cpp ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon jsoncpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "Using C++${CMAKE_CXX_STANDARD}")
