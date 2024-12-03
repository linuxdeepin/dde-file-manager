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

    add_library(${PROJECT_NAME}
        SHARED
        ${SRC_FILES}
    )
    target_link_libraries(${PROJECT_NAME}
        DFM::framework
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

    #install library file
    install(TARGETS
        ${PROJECT_NAME}
        LIBRARY
        DESTINATION
        ${DFM_PLUGIN_DESKTOP_CORE_DIR}
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


    # Qt6
    if (DFM_BUILD_WITH_QT6)
        include(ddplugin-wallpapersetting-qt6.cmake)
    else()
        include(ddplugin-wallpapersetting-qt5.cmake)
    endif()

endif()
