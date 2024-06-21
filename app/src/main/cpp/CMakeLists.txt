cmake_minimum_required(VERSION 3.22.1)

project("audioplayer")

set(ffmpeg_lib_dir ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
set(ffmpeg_head_dir ${CMAKE_SOURCE_DIR}/include)

include_directories(${ffmpeg_head_dir})

find_library(
        log-lib
        log
)

add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libavcodec-61.so)

add_library(libavdevice SHARED IMPORTED)
set_target_properties(libavdevice PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libavdevice-61.so)

add_library(libavfilter SHARED IMPORTED)
set_target_properties(libavfilter PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libavfilter-10.so)

add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libavformat-61.so)

add_library(libavutil SHARED IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libavutil-59.so)

add_library(libpostproc SHARED IMPORTED)
set_target_properties(libpostproc PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libpostproc-58.so)

add_library(libswresample SHARED IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libswresample-5.so)

add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_lib_dir}/libswscale-8.so)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        WlAudio.cpp
        WlCallJava.cpp
        WlFFmpeg.cpp
        WlPlaystatus.cpp
        WlQueue.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libpostproc
        libswresample
        libswscale
        OpenSLES
        log
        ${log-lib}
)