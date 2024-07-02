cmake_minimum_required(VERSION 3.10)

project(dfmplugin-filepreview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEPREVIEW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEPREVIEW_FILES}
)


add_definitions(-DPLUGINDIR=\"${DFM_PLUGIN_PREVIEW_DIR}\")

set(VERSION 1.8.2)
add_definitions(-DVERSION=\"${VERSION}\")

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_EDGE_DIR}
)

INSTALL_DCONFIG("org.deepin.dde.file-manager.preview.json")
