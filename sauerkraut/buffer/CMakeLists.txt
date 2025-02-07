# Find Flatbuffers package
find_package(flatbuffers REQUIRED)

# Get all .fbs files
file(GLOB FlatbufFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.fbs")

# Find flatc executable
if(DEFINED FLATC_EXECUTABLE)
    set(FLATC ${FLATC_EXECUTABLE})
else()
    find_program(FLATC flatc PATHS ${CMAKE_BINARY_DIR}/flatbuffers-install/bin NO_DEFAULT_PATH)
    if(NOT FLATC)
        find_program(FLATC flatc)
    endif()
endif()

if(NOT FLATC)
    message(FATAL_ERROR "flatc compiler not found")
endif()

# Create a custom command to generate C++ headers from Flatbuffer schemas
set(FLATBUFFER_GENERATED_HEADERS "")
foreach(FlatbufFile ${FlatbufFiles})
    get_filename_component(BASENAME ${FlatbufFile} NAME_WE)
    set(GENERATED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}_generated.h")
    add_custom_command(
        OUTPUT ${GENERATED_HEADER}
        COMMAND ${FLATC}
        ARGS --cpp -o "${CMAKE_CURRENT_BINARY_DIR}" ${FlatbufFile}
        DEPENDS ${FlatbufFile}
        COMMENT "Generating C++ header for ${BASENAME}"
    )
    list(APPEND FLATBUFFER_GENERATED_HEADERS ${GENERATED_HEADER})
endforeach()

message(STATUS "Flatbuffers generated headers: ${FLATBUFFER_GENERATED_HEADERS}")
# Create a custom target for generation
add_custom_target(generate_flatbuffers DEPENDS ${FLATBUFFER_GENERATED_HEADERS})
target_include_directories(sauerkraut PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${FLATBUFFER_INCLUDE_DIRS} 
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           )
