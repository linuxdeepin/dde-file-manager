cmake_minimum_required(VERSION 3.10)

project(serverplugin-grandsearchdaemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
)
