cmake_minimum_required(VERSION 3.10)

project(ddplugin-wallpapersetting)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(EXT_FILES
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
    )

# 指定资源文件
set(QRCS
    images.qrc
)

set(BIN_NAME dd-wallpapersetting-plugin)

# Include plugin configuration
include(DFMPluginConfig)

add_library(${BIN_NAME}
    SHARED
    ${SRC_FILES}
    ${EXT_FILES}
    ${QRCS}
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
dfm_setup_desktop_wallpapersetting_dependencies(${BIN_NAME})

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)
