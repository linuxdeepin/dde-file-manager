cmake_minimum_required(VERSION 3.10)

project(ddplugin-canvas)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../desktoputils/widgetutil.h
    )

FILE(GLOB_RECURSE CANVAS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5Svg REQUIRED)
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )

add_library(${PROJECT_NAME}
    SHARED
    ${CANVAS_FILES}
    ${EXT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${DtkGui_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    Qt5::Svg
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)
# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.sys-watermask.json")
