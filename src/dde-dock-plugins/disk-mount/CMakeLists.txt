cmake_minimum_required(VERSION 3.1.0)

project(dde-disk-mount-plugin)

#add_definitions(DFM_NO_FILE_WATCHER)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DBUS_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../../dfm-base/dbusservice)
set(DBUS_INTERFACES_DIR  ${DBUS_DIR}/dbus_interface)
include_directories(${DBUS_DIR})

set(MOUNT_PLUGIN_SRC
    disk-mount.json
    diskmountplugin.cpp
    tipswidget.cpp
    diskpluginitem.cpp
    diskcontrolwidget.cpp
    diskcontrolitem.cpp
    dattachedblockdevice.cpp
    dattachedprotocoldevice.cpp
    dattacheddevice.cpp
    pluginsidecar.cpp
    )

set(MOUNT_PLUGIN_HEADER
    diskmountplugin.h
    tipswidget.h
    diskpluginitem.h
    diskcontrolwidget.h
    diskcontrolitem.h
    dattachedblockdevice.h
    dattachedprotocoldevice.h
    dattacheddevice.h
    sizeformathelper.h
    pluginsidecar.h
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
find_package(DtkWidget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(Gesttings REQUIRED  gsettings-qt)

add_library(${PROJECT_NAME}
    SHARED
    ${MOUNT_PLUGIN_SRC}
    ${MOUNT_PLUGIN_HEADER}
    ${DBUS_INTERFACES_SRC}
    ${DBUS_INTERFACES_HEADER}
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
)

set(CMAKE_INSTALL_PREFIX "/usr")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)

# TODO(zhangs): translation
