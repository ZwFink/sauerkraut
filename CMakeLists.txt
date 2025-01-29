cmake_minimum_required(VERSION 3.15...3.30)
project(sauerkraut VERSION 0.1
        DESCRIPTION "Sauerkraut: Serialization for Python's Control State" 
        LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type and compiler flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall")

# Find Python (ensure the version matches your Python version)
find_package(Python 3.13 COMPONENTS Interpreter Development REQUIRED)

# Find Protobuf
find_package(FlatBuffers REQUIRED)

# Include directories (your source directory)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sauerkraut/include)


include_directories(${FlatBuffersIncludePath})
add_subdirectory(sauerkraut)

