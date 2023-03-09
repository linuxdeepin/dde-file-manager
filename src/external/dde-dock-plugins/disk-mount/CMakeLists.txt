cmake_minimum_required(VERSION 3.10)

project(dde-disk-mount-plugin)

#add_definitions(DFM_NO_FILE_WATCHER)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE (GLOB MOUNT_PLUGIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
include_directories(${SRC_DIR})
set(BASE_DEV_DIR ${SRC_DIR}/dfm-base/base/device)
FILE (GLOB BASE_DEVICE_FILES
    ${BASE_DEV_DIR}/*.h
    ${BASE_DEV_DIR}/*.cpp
    ${BASE_DEV_DIR}/private/*.h
    ${BASE_DEV_DIR}/private/*.cpp
)

set(BASE_UTIL_DIR ${SRC_DIR}/dfm-base/utils)
FILE(GLOB BASE_UTIL_FILES
    ${BASE_UTIL_DIR}/networkutils.h
    ${BASE_UTIL_DIR}/networkutils.cpp
    ${BASE_UTIL_DIR}/finallyutil.h
    ${BASE_UTIL_DIR}/finallyutil.cpp
)

set(DBUS_DIR ${SRC_DIR}/dfm-base/dbusservice)
include_directories(${DBUS_DIR})
FILE (GLOB DBUS_INTERFACE_FILES
    ${DBUS_DIR}/*.h
    ${DBUS_DIR}/*/devicemanagerdbus_interface.h
    ${DBUS_DIR}/*/devicemanagerdbus_interface.cpp
)

set(DBUS_INTERFACES_SRC
    ${DBUS_INTERFACES_DIR}/devicemanagerdbus_interface.cpp
    )

set(DBUS_INTERFACES_HEADER
    ${DBUS_INTERFACES_DIR}/devicemanagerdbus_interface.h
    ${DBUS_DIR}/global_server_defines.h
    )

find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(Gesttings REQUIRED  gsettings-qt)
pkg_search_module(dfm-mount REQUIRED dfm-mount IMPORTED_TARGET)

set(QRC_FILE
    resources.qrc
    resources/theme-icons.qrc
    )
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

add_library(${PROJECT_NAME}
    SHARED
    ${MOUNT_PLUGIN_FILES}
    ${BASE_DEVICE_FILES}
    ${BASE_UTIL_FILES}
    ${DBUS_INTERFACE_FILES}
    ${QRC_RESOURCES}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${Gesttings_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Gesttings_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
    PkgConfig::dfm-mount
    Qt5::Concurrent
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)

# schemas
install(FILES com.deepin.dde.dock.module.disk-mount.gschema.xml DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")

