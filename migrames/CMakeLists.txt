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
target_include_directories(migrames PUBLIC
    ${FlatBuffersIncludePath}
    ${FLATBUFFER_GENERATED_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
add_subdirectory(buffer)
add_subdirectory(serdes)

add_dependencies(migrames generate_flatbuffers)
# Link necessary libraries
target_link_libraries(migrames
    Python::Python
    pthread
    m
    util
    dl
    serdes
)

# shared library shoudl go to the build directory
set_target_properties(migrames PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
