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

    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Svg)
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(xcb REQUIRED xcb IMPORTED_TARGET)

    if(DFM_BUILD_WITH_QT6)
        set(INCLUDE_DIRS
            ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
        )
        set(EXTRA_LINK_LIBS "")
    else()
        pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
        set(INCLUDE_DIRS
            ${DFrameworkDBus_INCLUDE_DIRS}
            ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
        )
        set(EXTRA_LINK_LIBS
            ${DFrameworkDBus_LIBRARIES}
        )
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        DFM::base
        DFM::framework
        Qt${QT_VERSION_MAJOR}::Svg
        PkgConfig::xcb
        ${EXTRA_LINK_LIBS}
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
