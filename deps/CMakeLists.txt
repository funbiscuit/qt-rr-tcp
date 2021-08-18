add_library(binn STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/binn/src/binn.c
        )

target_include_directories(binn PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/binn/src
        )
