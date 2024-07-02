cmake_minimum_required(VERSION 3.10)

project(dfmplugin-burn)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB BURN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/menus/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menus/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${BURN_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

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

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.burn.json")
