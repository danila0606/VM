cmake_minimum_required(VERSION 3.10)
project(SadVM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include_directories(../vm/inc)
include_directories(../assembler/inc)
include_directories(../FileFormat)

file(GLOB CPP_SOURCES
     "../vm/src/*.cpp"
     "../FileFormat/*.cpp"
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES})