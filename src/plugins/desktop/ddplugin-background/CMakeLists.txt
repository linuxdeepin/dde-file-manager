cmake_minimum_required(VERSION 3.10)

project(ddplugin-background)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

#delete files which are not needed on the current system
FILE(GLOB_RECURSE RM_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.cpp"
    )

list(REMOVE_ITEM SRC_FILES ${RM_SRC})

set(BIN_NAME dd-background-plugin)

find_package(Dtk6 COMPONENTS Widget REQUIRED)

add_library(${BIN_NAME} SHARED ${SRC_FILES})

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_include_directories(${BIN_NAME} PUBLIC
    ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
)

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

