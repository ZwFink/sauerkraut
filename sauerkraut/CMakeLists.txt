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

# Find greenlet library path automatically
execute_process(
    COMMAND ${Python_EXECUTABLE} -c "import greenlet, os; base_dir = os.path.dirname(greenlet.__file__); print(base_dir); print([f for f in os.listdir(base_dir) if f.startswith('_greenlet.cpython-')][0])"
    OUTPUT_VARIABLE PYTHON_INFO
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Split the output into lines
string(REPLACE "\n" ";" PYTHON_INFO_LIST "${PYTHON_INFO}")
list(GET PYTHON_INFO_LIST 0 GREENLET_DIR)
list(GET PYTHON_INFO_LIST 1 GREENLET_FILENAME)

set(GREENLET_LIBRARY "${GREENLET_DIR}/${GREENLET_FILENAME}")

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
    "-Wl,--no-as-needed,${GREENLET_LIBRARY}"
)


# shared library shoudl go to the build directory
set_target_properties(sauerkraut PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
