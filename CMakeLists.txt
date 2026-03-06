cmake_minimum_required(VERSION 3.13)
project(shellout-project)

find_library(READLINE_LIB readline)

file(GLOB_RECURSE SOURCE_FILES src/*.c)
file(GLOB_RECURSE HEADER_FILES src/*.h)

set(CMAKE_C_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
add_executable(shellout ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(shellout PRIVATE ${READLINE_LIB})