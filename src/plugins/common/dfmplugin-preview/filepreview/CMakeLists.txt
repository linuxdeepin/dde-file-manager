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
find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEPREVIEW_FILES}
)

set(PREVIEWLIBPATH ${DFM_PLUGIN_DIR}/previews)
add_definitions(-DPLUGINDIR=\"${PREVIEWLIBPATH}\")

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
    ${DFM_PLUGIN_DIR}
)
