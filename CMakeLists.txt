cmake_minimum_required(VERSION 3.21)
project(video_app)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/libs/Linux)
message(STATUS ${CMAKE_PREFIX_PATH})
set(CMAKE_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/libs/Linux)
message(STATUS ${CMAKE_FRAMEWORK_PATH})

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/libs/Linux)



find_package(PkgConfig REQUIRED)

#message(STATUS ${PKG_CONFIG_EXECUTABLE})
#set(PKG_CONFIG_EXECUTABLE "${PKG_CONFIG_EXECUTABLE} --define-variable=CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
#message(STATUS ${PKG_CONFIG_EXECUTABLE})


pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libswresample
        libswscale
        )

add_executable(video_app src/main.cpp src/ScreenRecorder.cpp src/ScreenRecorder.h)

target_link_libraries(${PROJECT_NAME}
        PkgConfig::LIBAV
        )

