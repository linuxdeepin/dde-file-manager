cmake_minimum_required(VERSION 3.10)

project(ddplugin-wallpapersetting)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../ddplugin-background/backgrounddefault.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../ddplugin-background/backgrounddefault.cpp
    )

# 指定资源文件
set(QRCS
    images.qrc
)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
    ${EXT_FILES}
    ${QRCS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGui_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::desktop
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
