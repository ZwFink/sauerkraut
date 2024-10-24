# Source files
set(SOURCES
    mymodule.c
)

# Ensure the .c file is treated as C++ code
set_source_files_properties(mymodule.c PROPERTIES LANGUAGE CXX)
# Create the shared library (Python module)
add_library(mymodule SHARED ${SOURCES})
# Set the output properties to match Python module naming conventions
set_target_properties(mymodule PROPERTIES
    OUTPUT_NAME "mymodule"
    PREFIX ""
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)
# Include directories for the target
target_include_directories(mymodule PRIVATE
    ${Python_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)
add_subdirectory(buffer)

# Link necessary libraries
target_link_libraries(mymodule
    Python::Python
    ${Protobuf_LIBRARIES}
    pthread
    m
    util
    dl
    messages
)

# shared library shoudl go to the build directory
set_target_properties(mymodule PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
