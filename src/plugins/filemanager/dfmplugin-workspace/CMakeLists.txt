cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-workspace)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# TODO(zhangs): remove private
FILE(GLOB WORKSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${WORKSPACE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    DFMSERVICE::filemanager
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
