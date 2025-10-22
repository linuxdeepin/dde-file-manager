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

# Include plugin configuration
include(DFMPluginConfig)

add_library(${BIN_NAME} SHARED ${SRC_FILES})

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

# Use default plugin configuration
dfm_setup_desktop_background_dependencies(${BIN_NAME})

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

