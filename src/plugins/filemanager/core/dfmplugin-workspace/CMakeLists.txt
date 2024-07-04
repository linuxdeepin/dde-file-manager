cmake_minimum_required(VERSION 3.10)

project(dfmplugin-workspace)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# TODO(zhangs): remove private
FILE(GLOB WORKSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/menus/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menus/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${WORKSPACE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
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
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
