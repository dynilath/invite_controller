cmake_minimum_required(VERSION 3.9)
project(invite_controller)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

if(NOT DEFINED _MASTER_ID OR _MASTER_ID EQUAL "")
    message(FATAL_ERROR "_MASTER_ID must be defined")
else()
    add_definitions(-DMASTER_ID=${_MASTER_ID})
    message("MASTER_ID is ${_MASTER_ID}")
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_link_options("$<$<CXX_COMPILER_ID:MSVC>:/export:apprun=_apprun@8>")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.cc)
file(GLOB_RECURSE CORN_SOURCE extern/corncppsdk/**.cc)

set(LIB_NAME "controller")
add_library(${LIB_NAME} SHARED ${SOURCE_FILES} ${CORN_SOURCE})
target_link_options(${LIB_NAME} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/export:apprun=_apprun@8>")

