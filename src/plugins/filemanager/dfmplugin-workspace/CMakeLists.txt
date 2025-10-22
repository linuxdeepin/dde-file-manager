cmake_minimum_required(VERSION 3.10)

project(dfmplugin-workspace)
set(BIN_NAME dfm-workspace-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include shared dependencies configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

FILE(GLOB_RECURSE WORKSPACE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${BIN_NAME}
    SHARED
    ${WORKSPACE_FILES}
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
target_link_libraries(${BIN_NAME}
    PUBLIC
    Qt6::Core
    Qt6::Widgets
)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
    find_package(Qt6 REQUIRED COMPONENTS WidgetsPrivate)
    target_link_libraries(${BIN_NAME} PRIVATE Qt6::WidgetsPrivate)
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Apply workspace plugin specific configuration
dfm_setup_workspace_dependencies(${BIN_NAME})

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
