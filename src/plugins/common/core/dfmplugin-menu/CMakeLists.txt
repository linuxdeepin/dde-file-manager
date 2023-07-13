cmake_minimum_required(VERSION 3.10)

project(dfmplugin-menu)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE MENU_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${MENU_FILES}
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

target_compile_definitions(${PROJECT_NAME} PRIVATE MENU_CHECK_FOCUSONLY)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
