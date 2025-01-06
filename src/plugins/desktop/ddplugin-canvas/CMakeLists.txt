cmake_minimum_required(VERSION 3.10)

project(ddplugin-canvas)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
    )

FILE(GLOB_RECURSE CANVAS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(BIN_NAME dd-canvas-plugin)

find_package(Qt6 COMPONENTS Core Widgets Gui Svg REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${CANVAS_FILES}
    ${EXT_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_include_directories(${BIN_NAME} PUBLIC
    ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Svg
)

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.sys-watermask.json")
