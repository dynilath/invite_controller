cmake_minimum_required(VERSION 3.9)
project(invite_controller)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

include(extern/cqcppsdk/cqcppsdk.cmake)
include_directories(src)

file(READ "app_id.txt" APP_ID)
string(STRIP "${APP_ID}" APP_ID)
cq_set_app_id(${APP_ID})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

if (CQ_CAN_BUILD_STD_MODE)
    set(CQCPPSDK_DEV_MODE OFF)
    cq_add_app(app ${SOURCE_FILES}) 
endif ()

cq_add_dev_app(${SOURCE_FILES})
