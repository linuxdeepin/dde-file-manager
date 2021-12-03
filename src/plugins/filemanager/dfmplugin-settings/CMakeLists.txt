cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-settings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB  GLOBAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB PRIVATE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/private/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp
)

find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${PRIVATE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
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
