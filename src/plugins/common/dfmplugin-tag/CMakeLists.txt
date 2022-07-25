cmake_minimum_required(VERSION 3.10)

project(dfmplugin-tag)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB TAG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/files/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/files/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/files/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/files/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${TAG_FILES}
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
