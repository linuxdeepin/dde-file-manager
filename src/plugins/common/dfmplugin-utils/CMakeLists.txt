cmake_minimum_required(VERSION 3.10)

project(dfmplugin-utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE UTILS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS Svg REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${UTILS_FILES}
)

set(EXTENSIONS_LIB_DIR ${DFM_PLUGIN_DIR}/extensions)
add_definitions(-DEXTENSIONS_PATH=\"${EXTENSIONS_LIB_DIR}\")
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFM::extension
    Qt5::Svg
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_EDGE_DIR}
)
