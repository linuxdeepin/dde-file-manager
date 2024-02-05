cmake_minimum_required(VERSION 3.10)

project(dfmplugin-propertydialog)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QRC_FILES
    qrc/images.qrc
    )

qt5_add_resources(QRC_RESOURCES ${QRC_FILES})

FILE(GLOB DETAILSPCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../services/filemanager/)

find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${DETAILSPCE_FILES}
    ${QRC_RESOURCES}
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
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
