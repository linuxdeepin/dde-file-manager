cmake_minimum_required(VERSION 3.10)

project(dde-disk-mount-plugin)

#add_definitions(DFM_NO_FILE_WATCHER)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE (GLOB_RECURSE MOUNT_PLUGIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice/global_server_defines.h"
)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5 COMPONENTS Concurrent Core REQUIRED)
find_package(Dtk COMPONENTS Widget Gui REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(Gesttings REQUIRED gsettings-qt)
pkg_search_module(LibMount REQUIRED mount IMPORTED_TARGET)

set(QRC_FILE
    resources.qrc
    resources/theme-icons.qrc
)
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

qt5_add_dbus_interface(MOUNT_PLUGIN_FILES
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.Daemon.DeviceManager.xml"
    devicemanager_interface
)

add_library(${PROJECT_NAME}
    SHARED
    ${MOUNT_PLUGIN_FILES}
    ${QRC_RESOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGui_INCLUDE_DIRS}
    ${Gesttings_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/dfm-base/dbusservice
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    ${Gesttings_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
    PkgConfig::LibMount
    Qt5::Concurrent
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)

# schemas
install(FILES com.deepin.dde.dock.module.disk-mount.gschema.xml DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")
