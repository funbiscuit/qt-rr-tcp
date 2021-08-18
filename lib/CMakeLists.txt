if (WIN32)
    set(QT_WIN_PATH "C:\\" CACHE STRING "Path to installed qt binaries")
    message(STATUS "Looking for installed Qt5 at the following path:\n" ${QT_WIN_PATH})
    set(CMAKE_PREFIX_PATH ${QT_WIN_PATH})
endif ()

find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_library(qt_rr_tcp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RRTcpServer.cpp
        )

target_compile_features(qt_rr_tcp PUBLIC cxx_std_11)

set_target_properties(qt_rr_tcp PROPERTIES
        CXX_EXTENSIONS OFF
        AUTOMOC ON)

target_include_directories(qt_rr_tcp
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

target_link_libraries(qt_rr_tcp PUBLIC Qt5::Network binn)
