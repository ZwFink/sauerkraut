set(SOURCES
    greenlet_compat.C
    include/greenlet_compat.h
)

# Execute Python script to find greenlet installation path
execute_process(
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/find_greenlet.py
    OUTPUT_VARIABLE GREENLET_INCLUDE_DIR
    RESULT_VARIABLE FIND_GREENLET_RESULT
)

if(NOT FIND_GREENLET_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to find greenlet installation directory")
endif()

add_library(greenlet_compat SHARED ${SOURCES})

set_target_properties(greenlet_compat PROPERTIES
    OUTPUT_NAME "greenlet_compat"
    PREFIX ""
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

target_include_directories(greenlet_compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Python_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/sauerkraut/include
    ${GREENLET_INCLUDE_DIR}
)