cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

set(PROJECT_NAME webserver)

project(${PROJECT_NAME})

file(GLOB SOURCES "source/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)