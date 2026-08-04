cmake_minimum_required(VERSION 3.10)

project(test-env-monitor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Widgets DBus REQUIRED)
find_package(Dtk6 COMPONENTS Core Widget REQUIRED)
find_package(dfm6-search REQUIRED)
find_package(dfm6-base REQUIRED)
find_package(KF6IdleTime REQUIRED)

FILE(GLOB_RECURSE TEST_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${TEST_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Dtk6::Core
    Dtk6::Widget
    dde-filemanager-textindex
    dfm6-search
    dfm6-base
    KF6::IdleTime
)

if(EXISTS "${DFM_APP_SOURCE_DIR}/config.h.in")
    configure_file(
        "${DFM_APP_SOURCE_DIR}/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DFM_PROJECT_ROOT}/src/services/textindex
    ${DFM_PROJECT_ROOT}/include
    ${DFM_PROJECT_ROOT}/3rdparty
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

message(STATUS "DFM: env-monitor-test configured")
