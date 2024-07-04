cmake_minimum_required(VERSION 3.10)

project(ddplugin-wallpapersetting)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (COMPLIE_ON_V20)
set(SRC_FILES
    ddplugin_wallpapersetting_global.h
    wlsetplugin.h
    wlsetplugin.cpp
    settingsdbusinterface.h
    settingsdbusinterface.cpp
    wallpapersetting.json
)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)
target_link_libraries(${PROJECT_NAME}
    DFM::framework
)

else()

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

#delete files which are not needed on the current system
if (COMPLIE_ON_V20)
    FILE(GLOB_RECURSE RM_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.cpp"
        )
endif()

list(REMOVE_ITEM SRC_FILES ${RM_SRC})

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.h
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

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Svg)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )
pkg_check_modules(xcb REQUIRED xcb IMPORTED_TARGET)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGui_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    Qt5::Svg
    PkgConfig::xcb
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_EDGE_DIR}
)
