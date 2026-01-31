cmake_minimum_required(VERSION 3.16)

project(dsl_poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    include/*.hpp
)

add_library(dsl_poc_obj OBJECT
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(dsl_poc_obj
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(dsl_poc_lib
    $<TARGET_OBJECTS:dsl_poc_obj>
)

add_executable(dsl_poc_exec
    $<TARGET_OBJECTS:dsl_poc_obj>
)

