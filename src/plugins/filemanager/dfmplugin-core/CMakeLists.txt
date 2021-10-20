cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(core_cxx_cpp
    corebrowseview.cpp
    eventcaller.cpp
    corelog.cpp
    core.cpp
    core.json
    )

set(core_cxx_h
    corebrowseview.h
    eventcaller.h
    corelog.h
    core.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${core_cxx_cpp}
    ${core_cxx_h}
    )

target_link_libraries(${PROJECT_NAME}
    dfm-base
    dfm-framework
    dfmservice-common
    dfmservice-filemanager
    ${QtUseModules}
    ${PkgUserModules}
    )


