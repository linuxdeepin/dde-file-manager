cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/services/dfmservices-business/dfmservice-filemanager
    )

set(core_cxx_cpp
    eventcaller.cpp
    corelog.cpp
    core.cpp
    core.json
    )

set(core_cxx_h
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
    dfmservice-filemanager
    ${QtUseModules}
    ${PkgUserModules}
    )


