cmake_minimum_required(VERSION 3.10)

project(dfmplugin-smbbrowser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE BROWSER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${BROWSER_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Network
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)


#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_EDGE_DIR}
)
