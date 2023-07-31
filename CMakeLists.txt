cmake_minimum_required(VERSION 3.14)

project(memfs)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/include/memfs.h"
    "${PROJECT_SOURCE_DIR}/include/memfs-disk.h"
    "${PROJECT_SOURCE_DIR}/include/memfs-defs.h"

    "${PROJECT_SOURCE_DIR}/src/memfs.c"
    "${PROJECT_SOURCE_DIR}/src/memfs-crc.c"
    "${PROJECT_SOURCE_DIR}/src/memfs-util.c"

    "${PROJECT_SOURCE_DIR}/src/include/memfs-crc.h"
    "${PROJECT_SOURCE_DIR}/src/include/memfs-util.h"
    "${PROJECT_SOURCE_DIR}/src/include/memfs-private.h"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
