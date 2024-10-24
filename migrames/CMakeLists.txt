# Source files
set(SOURCES
    migrames.C
)

# Create the shared library (Python module)
add_library(migrames SHARED ${SOURCES})
# Set the output properties to match Python module naming conventions
set_target_properties(migrames PROPERTIES
    OUTPUT_NAME "migrames"
    PREFIX ""
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)
# Include directories for the target
target_include_directories(migrames PRIVATE
    ${Python_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)
add_subdirectory(buffer)

# Link necessary libraries
target_link_libraries(migrames
    Python::Python
    ${Protobuf_LIBRARIES}
    pthread
    m
    util
    dl
    messages
)

# shared library shoudl go to the build directory
set_target_properties(migrames PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
