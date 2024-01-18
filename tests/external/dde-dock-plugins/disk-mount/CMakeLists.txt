cmake_minimum_required(VERSION 3.10)

project(disk-mount-demo)

#add_definitions(DFM_NO_FILE_WATCHER)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DOCK_SRC_PATH "${CMAKE_SOURCE_DIR}/src/external/dde-dock-plugins/disk-mount")
FILE (GLOB_RECURSE DOCK_FILES
    "${DOCK_SRC_PATH}/device/*.h"
    "${DOCK_SRC_PATH}/device/*.cpp"
    "${DOCK_SRC_PATH}/utils/*.h"
    "${DOCK_SRC_PATH}/utils/*.cpp"
    "${DOCK_SRC_PATH}/widgets/deviceitem.h"
    "${DOCK_SRC_PATH}/widgets/deviceitem.cpp"
    "${DOCK_SRC_PATH}/widgets/devicelist.h"
    "${DOCK_SRC_PATH}/widgets/devicelist.cpp"
    "${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice/global_server_defines.h"
    main.cpp
)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5 COMPONENTS Concurrent Core REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(LibMount REQUIRED mount IMPORTED_TARGET)

set(QRC_FILE
    ${DOCK_SRC_PATH}/resources.qrc
    ${DOCK_SRC_PATH}/resources/theme-icons.qrc
    )
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

qt5_add_dbus_interface(DOCK_FILES
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.filemanager.server.DeviceManager.xml"
    devicemanager_interface)

add_executable(${PROJECT_NAME}
    ${DOCK_FILES}
    ${QRC_RESOURCES}
)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice
    ${DOCK_SRC_PATH}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    PkgConfig::LibMount
    Qt5::Concurrent
)
