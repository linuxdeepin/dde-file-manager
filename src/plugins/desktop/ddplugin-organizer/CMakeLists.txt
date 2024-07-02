cmake_minimum_required(VERSION 3.10)

project(ddplugin-organizer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.h
    )

FILE(GLOB_RECURSE CORE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )

# 指定资源文件
set(QRC_FILE
    resources/images.qrc
)
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

add_library(${PROJECT_NAME}
    SHARED
    ${CORE_FILES}
    ${EXT_FILES}
    ${QRC_RESOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${DtkGui_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_EDGE_DIR}
    )

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.organizer.json")
