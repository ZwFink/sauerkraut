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
set(FlatBuffersIncludePath ${FLATBUFFER_INCLUDE_DIRS})
# Include directories for the target
target_include_directories(migrames PRIVATE
    ${FlatBuffersIncludePath}
    ${FLATBUFFER_GENERATED_HEADERS}
    )
add_subdirectory(buffer)

add_dependencies(migrames generate_flatbuffers)
# Link necessary libraries
target_link_libraries(migrames
    Python::Python
    ${Protobuf_LIBRARIES}
    pthread
    m
    util
    dl
)

# shared library shoudl go to the build directory
set_target_properties(migrames PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
