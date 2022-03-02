cmake_minimum_required(VERSION 3.10)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../desktoputils/widgetutil.cpp
    )

set(CORE_FILES
    frame/basewindow.h
    frame/basewindow.cpp
    frame/windowframe_p.h
    frame/windowframe.h
    frame/windowframe.cpp
    ddplugin_core_global.h
    core.h
    core.cpp
    core.json
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CORE_FILES}
    ${EXT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::desktop
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
