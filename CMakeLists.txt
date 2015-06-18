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

target_link_libraries(${PROJECT_NAME}
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
endif()
