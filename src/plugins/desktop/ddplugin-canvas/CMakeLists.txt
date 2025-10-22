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

# Include plugin configuration
include(DFMPluginConfig)

add_library(${BIN_NAME}
    SHARED
    ${CANVAS_FILES}
    ${EXT_FILES}
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
target_link_libraries(${BIN_NAME} 
    PUBLIC
    Qt6::Core
    Qt6::Gui
)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(${BIN_NAME} PRIVATE Qt6::GuiPrivate)
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

# Include shared dependencies configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

# Apply plugin specific configuration
dfm_setup_desktop_canvas_dependencies(${BIN_NAME})

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.sys-watermask.json")
