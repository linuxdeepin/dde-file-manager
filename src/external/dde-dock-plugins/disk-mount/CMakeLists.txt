cmake_minimum_required(VERSION 3.10)

project(dde-disk-mount-plugin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE (GLOB_RECURSE MOUNT_PLUGIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice/global_server_defines.h"
)

find_package(Qt6 COMPONENTS Widgets DBus Concurrent Core REQUIRED)
find_package(Dtk6 COMPONENTS Widget Gui REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_search_module(LibMount REQUIRED mount IMPORTED_TARGET)

set(QRC_FILE
    resources.qrc
    resources/theme-icons.qrc
)
qt6_add_resources(QRC_RESOURCES ${QRC_FILE})

qt6_add_dbus_interface(MOUNT_PLUGIN_FILES
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.Daemon.DeviceManager.xml"
    devicemanager_interface
)

add_library(${PROJECT_NAME} SHARED
    ${MOUNT_PLUGIN_FILES}
    ${QRC_RESOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DdeDockInterface_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${DdeDockInterface_LIBRARIES}
    Qt6::Widgets
    Qt6::Core
    Qt6::DBus
    Qt6::Concurrent
    Dtk6::Widget
    Dtk6::Gui
    PkgConfig::LibMount
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)
