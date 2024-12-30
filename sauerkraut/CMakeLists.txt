# Source files
set(SOURCES
    sauerkraut.C
)

# Create the shared library (Python module)
add_library(sauerkraut SHARED ${SOURCES})
# Set the output properties to match Python module naming conventions
set_target_properties(sauerkraut PROPERTIES
    OUTPUT_NAME "sauerkraut"
    PREFIX ""
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)
set(FlatBuffersIncludePath ${FLATBUFFER_INCLUDE_DIRS})
# Include directories for the target
target_include_directories(sauerkraut PUBLIC
    ${FlatBuffersIncludePath}
    ${FLATBUFFER_GENERATED_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
add_subdirectory(buffer)
add_subdirectory(serdes)

add_dependencies(sauerkraut generate_flatbuffers serdes)
# Link necessary libraries
target_link_libraries(sauerkraut
    Python::Python
    pthread
    m
    util
    dl
    serdes
)

# shared library shoudl go to the build directory
set_target_properties(sauerkraut PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
