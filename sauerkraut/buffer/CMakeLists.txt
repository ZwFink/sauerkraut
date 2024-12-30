# Find Flatbuffers package
find_package(Flatbuffers REQUIRED)

# Get all .fbs files
file(GLOB FlatbufFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.fbs")

# Create a custom command to generate C++ headers from Flatbuffer schemas
set(FLATBUFFER_GENERATED_HEADERS "")
foreach(FlatbufFile ${FlatbufFiles})
    get_filename_component(BASENAME ${FlatbufFile} NAME_WE)
    set(GENERATED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}_generated.h")
    add_custom_command(
        OUTPUT ${GENERATED_HEADER}
        COMMAND flatc
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
