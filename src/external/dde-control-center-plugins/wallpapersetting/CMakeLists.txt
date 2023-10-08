cmake_minimum_required(VERSION 3.10)

project(wallpapersetting)

set(WALLPAPER_NAME dcc-wallpapersetting-plugin)
set(SCREENSAVER_NAME dcc-screensaversetting-plugin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

FILE (GLOB_RECURSE COMMON_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

FILE (GLOB_RECURSE WALLPAPER_SRC
    wallpapersettingplugin.h
    wallpapersettingplugin.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/wallpaper/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/wallpaper/*.cpp"
)

FILE (GLOB_RECURSE SCREENSAVER_SRC
    screensaversettingplugin.h
    screensaversettingplugin.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/screensaver/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screensaver/*.cpp"
)

find_package(Qt5 COMPONENTS Core Widgets DBus LinguistTools REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )
pkg_check_modules(QGSet REQUIRED gsettings-qt)

add_library(${WALLPAPER_NAME} SHARED
    ${WALLPAPER_SRC}
    ${COMMON_SRC}
    translation.qrc
    wallpaper-resources.qrc
    )

add_library(${SCREENSAVER_NAME} SHARED
    ${SCREENSAVER_SRC}
    ${COMMON_SRC}
    translation.qrc
    screensaver-resources.qrc
    )

#set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)
include_directories(
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${DdeControlCenter_INCLUDE_DIR}
    ${DtkGui_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    )

link_libraries(
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    ${DdeControlCenter_LIBRARIES}
)

target_include_directories(${SCREENSAVER_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop
    ${QGSet_INCLUDE_DIRS}
)

target_link_libraries(${SCREENSAVER_NAME} PRIVATE
    ${QGSet_LIBRARIES}
)

file(GLOB TS_FILES "translations/*.ts")
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION share/${WALLPAPER_NAME}/translations)

install(TARGETS ${WALLPAPER_NAME} LIBRARY DESTINATION lib/dde-control-center/modules)
install(TARGETS ${SCREENSAVER_NAME} LIBRARY DESTINATION lib/dde-control-center/modules)
