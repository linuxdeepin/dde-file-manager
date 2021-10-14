cmake_minimum_required(VERSION 3.1.0)

project(dfmplugin-interfaces)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(INTERFACES_H
    interfaces.h
    )

set(INTERFACES_CPP
    interfaces.cpp
    interfaces.json
    )

add_library(${PROJECT_NAME}
    SHARED
    ${INTERFACES_H}
    ${INTERFACES_CPP}
    )

target_link_libraries(${PROJECT_NAME}
    dfm-framework
    dfmservice-common
    dfm-base
    ${QtUseModules}
    ${PkgUserModules}
    )
