cmake_minimum_required(VERSION 3.10)

project(ddplugin-organizer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
    )

FILE(GLOB_RECURSE CORE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(BIN_NAME dd-organizer-plugin)

# Include plugin configuration
include(DFMPluginConfig)

# 指定资源文件
set(QRC_FILE
    resources/images.qrc
)

find_package(Dtk6 COMPONENTS Gui REQUIRED)

qt_add_resources(QRC_RESOURCES ${QRC_FILE})

add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
    ${EXT_FILES}
    ${QRC_RESOURCES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_link_libraries(${BIN_NAME} PRIVATE
    DFM6::base
    DFM6::framework
    Dtk6::Gui
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

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.organizer.json")
