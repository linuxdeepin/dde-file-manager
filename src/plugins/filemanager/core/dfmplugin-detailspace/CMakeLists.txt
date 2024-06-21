cmake_minimum_required(VERSION 3.10)

project(dfmplugin-detailspace)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE DETAILSPCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qml"
)

find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${DETAILSPCE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::gui
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

#Install qml file
INSTALL_PLUGIN_QUICK_MODULE(${DFM_PLUGIN_FILEMANAGER_CORE_DIR})
