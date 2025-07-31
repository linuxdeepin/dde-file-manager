cmake_minimum_required(VERSION 3.10)

project(disk-mount-demo)

#add_definitions(DFM_NO_FILE_WATCHER)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DFM_ROOT_PATH "${CMAKE_SOURCE_DIR}/../..")
set(DFM_SRC_PATH "${DFM_ROOT_PATH}/src")
set(DOCK_SRC_PATH "${DFM_SRC_PATH}/external/dde-dock-plugins/disk-mount")
FILE (GLOB_RECURSE DOCK_FILES
    "${DOCK_SRC_PATH}/*.h"
    "${DOCK_SRC_PATH}/*.cpp"
    "${DOCK_SRC_PATH}/*.json"
    "${DFM_SRC_PATH}/dfm-base/dbusservice/global_server_defines.h"
    main.cpp
)

find_package(Qt6 COMPONENTS Core Widgets DBus Concurrent REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(LibMount REQUIRED mount IMPORTED_TARGET)

set(QRC_FILE
    ${DOCK_SRC_PATH}/resources.qrc
    ${DOCK_SRC_PATH}/resources/theme-icons.qrc
    )
qt6_add_resources(QRC_RESOURCES ${QRC_FILE})

qt6_add_dbus_interface(DOCK_FILES
    "${DFM_ROOT_PATH}/assets/dbus/org.deepin.Filemanager.Daemon.DeviceManager.xml"
    devicemanager_interface)

add_executable(${PROJECT_NAME}
    ${DOCK_FILES}
    ${QRC_RESOURCES}
)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${DdeDockInterface_INCLUDE_DIRS}
    ${DFM_SRC_PATH}/dfm-base/dbusservice
    ${DOCK_SRC_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::Concurrent
    Dtk6::Widget
    PkgConfig::LibMount
)
