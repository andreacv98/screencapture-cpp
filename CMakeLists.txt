cmake_minimum_required(VERSION 3.20)
project(video_app)

set(CMAKE_CXX_STANDARD 14)

set (CMAKE_VERBOSE_MAKEFILE true)



message("System info following:")
message("OS version: ${CMAKE_SYSTEM_VERSION}; Processor: ${CMAKE_SYSTEM_PROCESSOR}; Host name: ${CMAKE_HOST_SYSTEM_NAME}")

if(WIN32)
    message("Windows OS detected")
    set(WINDOWS_PATH libs/Windows)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/${WINDOWS_PATH}")
    message(STATUS ${CMAKE_PREFIX_PATH})
    set(CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/${WINDOWS_PATH}")
    message(STATUS ${CMAKE_FRAMEWORK_PATH})

    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/${WINDOWS_PATH}")
    message(STATUS ${CMAKE_INSTALL_PREFIX})
elseif(APPLE)
    message("Apple OS detected")
elseif(UNIX)
    message("UNIX OS detected")

    set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/libs/Linux)
    message(STATUS ${CMAKE_PREFIX_PATH})
    set(CMAKE_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/libs/Linux)
    message(STATUS ${CMAKE_FRAMEWORK_PATH})

    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/libs/Linux)
    message(STATUS ${CMAKE_INSTALL_PREFIX})
else()
    message(FATAL_ERROR "!!NO OS DETECTED!!")
endif()

find_package(PkgConfig REQUIRED)

message(STATUS ${PKG_CONFIG_EXECUTABLE})
set(PKG_CONFIG_EXECUTABLE "${PKG_CONFIG_EXECUTABLE};--define-variable=CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message(STATUS ${PKG_CONFIG_EXECUTABLE})


pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libswresample
        libswscale
        )

add_executable(
        video_app
        src/main.cpp
        src/libs/Demuxer.cpp
        src/libs/Demuxer.h
        src/libs/VideoDemuxer.cpp
        src/libs/AudioDemuxer.cpp
        src/libs/AudioDemuxer.h
        src/libs/Decoder.h
        src/libs/Decoder.cpp
        src/libs/Encoder.h
        src/libs/Encoder.cpp
        src/libs/Muxer.h src/libs/Muxer.cpp
        src/libs/utils.h
        src/libs/Controller.cpp src/libs/Controller.h)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
            PkgConfig::LIBAV
            )
elseif(APPLE)
    message("Apple OS detected")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME}
            PkgConfig::LIBAV
            X11
            )
endif()

