cmake_minimum_required(VERSION 3.10)
project(Assembler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include_directories(../assembler/inc)
include_directories(../vm/inc)
include_directories(../FileFormat)

file(GLOB CPP_SOURCES
     #"../assembler/inc/*.hpp"
     "../assembler/src/*.cpp"
     #"../vm/inc/*.h"
     "../FileFormat/*.cpp"
     #"../vm/src/*.c"
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES})