if (WIN32)
    set(QT_WIN_PATH "C:\\" CACHE STRING "Path to installed qt binaries")
    message(STATUS "Looking for installed Qt5 at the following path:\n" ${QT_WIN_PATH})
    set(CMAKE_PREFIX_PATH ${QT_WIN_PATH})
endif ()

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(rr_example_client
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/task.cpp
        )

target_compile_features(rr_example_client PUBLIC cxx_std_11)

set_target_properties(rr_example_client PROPERTIES
        CXX_EXTENSIONS OFF
        AUTOMOC ON)

target_include_directories(rr_example_client
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

target_link_libraries(rr_example_client PRIVATE qt_rr_tcp)
