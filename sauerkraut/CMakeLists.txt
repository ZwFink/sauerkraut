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
add_subdirectory(greenlet_compat)
add_dependencies(sauerkraut generate_flatbuffers serdes greenlet_compat)

# Set the exact path to the greenlet library
set(GREENLET_LIBRARY "/cpython-install/lib/python3.13/site-packages/greenlet/_greenlet.cpython-313d-x86_64-linux-gnu.so")

if(NOT EXISTS ${GREENLET_LIBRARY})
    message(FATAL_ERROR "Greenlet library not found at ${GREENLET_LIBRARY}")
endif()

# Link necessary libraries
target_link_libraries(sauerkraut
    PRIVATE
    Python::Python
    pthread
    m
    util
    dl
    serdes
    greenlet_compat
    "-Wl,--no-as-needed,${GREENLET_LIBRARY}"  # Force linking with the full path
)

# Set RPATH to include the directory containing greenlet
set_target_properties(sauerkraut PROPERTIES
    INSTALL_RPATH "/cpython-install/lib/python3.13/site-packages/greenlet"
    BUILD_RPATH "/cpython-install/lib/python3.13/site-packages/greenlet"
    SKIP_BUILD_RPATH FALSE
)

# shared library shoudl go to the build directory
set_target_properties(sauerkraut PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
