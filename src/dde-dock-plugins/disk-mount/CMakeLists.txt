cmake_minimum_required(VERSION 3.1.0)

project(disk-mount)

#add_definitions(DFM_NO_FILE_WATCHER)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTOUIC ON)

# generate a compile commands file as complete database for vim-YouCompleteMe or some other similar tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MOUNT_PLUGIN_SRC
    diskmountplugin.cpp
    disk-mount.json
    )

set(MOUNT_PLUGIN_HEADER
    diskmountplugin.h
    )

find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)

add_library(${PROJECT_NAME}
    SHARED
    ${MOUNT_PLUGIN_SRC}
    ${MOUNT_PLUGIN_HEADER}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
)

set(CMAKE_INSTALL_PREFIX "/usr")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)

# TODO(zhangs): translation
