cmake_minimum_required(VERSION 3.10)

project(dfmplugin-optical)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE OPTICAL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Qt6 COMPONENTS DBus Widgets Concurrent SvgWidgets REQUIRED)
set(SVGLIB Qt6::Svg)

add_library(${PROJECT_NAME}
    SHARED
    ${OPTICAL_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::Concurrent
    Qt6::SvgWidgets
    DFM6::base
    DFM6::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
