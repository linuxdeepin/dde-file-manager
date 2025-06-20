cmake_minimum_required(VERSION 3.10)

project(ddplugin-wallpapersetting)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (COMPILE_ON_V20)
    set(SRC_FILES
        ddplugin_wallpapersetting_global.h
        wlsetplugin.h
        wlsetplugin.cpp
        settingsdbusinterface.h
        settingsdbusinterface.cpp
        wallpapersetting.json
    )

    set(BIN_NAME dd-wallpapersetting-plugin)

    add_library(${BIN_NAME}
        SHARED
        ${SRC_FILES}
    )
    target_link_libraries(${BIN_NAME}
        DFM6::framework
    )

    set_target_properties(${BIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

    install(TARGETS ${BIN_NAME}
        LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
    )

else()

    FILE(GLOB_RECURSE SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

    set(EXT_FILES
        ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
        )

    # 指定资源文件
    set(QRCS
        images.qrc
    )

    set(BIN_NAME dd-wallpapersetting-plugin)

    find_package(Qt6 COMPONENTS Core Svg REQUIRED)
    find_package(Dtk6 COMPONENTS Widget REQUIRED)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(xcb REQUIRED xcb IMPORTED_TARGET)

    add_library(${BIN_NAME}
        SHARED
        ${SRC_FILES}
        ${EXT_FILES}
        ${QRCS}
    )

    target_include_directories(${BIN_NAME} PUBLIC
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )

    target_link_libraries(${BIN_NAME}
        Qt6::Svg
        DFM6::base
        DFM6::framework
        PkgConfig::xcb
    )

    set_target_properties(${BIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

    install(TARGETS ${BIN_NAME}
        LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
    )

endif()
