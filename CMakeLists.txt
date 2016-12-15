cmake_minimum_required(VERSION 2.8)
project(fusecloop)

include(FindZLIB)
if (NOT ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB not found!")
endif()

file(READ "VERSION" FUSECLOOP_VERSION)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/strver "\"${FUSECLOOP_VERSION}\"")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    fusecloop.c
    cloopreader.c
    cloopreader.h
    debug.c
    debug.h
    common_header.h
    compressed_loop.h
)

add_executable(create_compressed_fs
    create_compressed_fs.c
)

add_executable(extract_compressed_fs
    extract_compressed_fs.c
)

target_link_libraries(${PROJECT_NAME}
    ${ZLIB_LIBRARIES}
)

target_link_libraries(create_compressed_fs
    ${ZLIB_LIBRARIES}
)

target_link_libraries(extract_compressed_fs
    ${ZLIB_LIBRARIES}
)

add_definitions(
    -Wall
    -Wextra
)

if (APPLE)
    add_definitions(
        -D_FILE_OFFSET_BITS=64
    )
    include_directories(
        /usr/local/include/osxfuse
    )
    target_link_libraries(${PROJECT_NAME}
        -L/usr/local/lib
        -losxfuse
    )
else()
    find_package(PkgConfig)
    pkg_search_module(FUSE REQUIRED fuse)
    target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${FUSE_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PUBLIC ${FUSE_CFLAGS_OTHER})
endif()
