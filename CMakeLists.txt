cmake_minimum_required(VERSION 3.14)
project(MyModule CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type and compiler flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall")

# Find Python (ensure the version matches your Python version)
find_package(Python 3.14 COMPONENTS Interpreter Development REQUIRED)

# Find Protobuf
find_package(Protobuf REQUIRED)

# Include directories (your source directory)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(SOURCES
    migrames/mymodule.c
)

# Ensure the .c file is treated as C++ code
set_source_files_properties(migrames/mymodule.c PROPERTIES LANGUAGE CXX)

# Create the shared library (Python module)
add_library(mymodule SHARED ${SOURCES})

# Set the output properties to match Python module naming conventions
set_target_properties(mymodule PROPERTIES
    OUTPUT_NAME "mymodule"
    PREFIX ""
)

# Include directories for the target
target_include_directories(mymodule PRIVATE
    ${Python_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)

# Link necessary libraries
target_link_libraries(mymodule
    Python::Python
    ${Protobuf_LIBRARIES}
    pthread
    m
    util
    dl
)
