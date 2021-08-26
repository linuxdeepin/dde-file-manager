cmake_minimum_required(VERSION 3.0.2)

project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/services/dfm-business-services/dfm-filemanager-service
    )

set(core_cxx_source
    core.cpp
    core.json
    )

set(core_cxx_include
    core.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${core_cxx_source}
    ${core_cxx_include}
    )

target_link_libraries(${PROJECT_NAME}
    dfm-base
    dfm-framework
    dfm-filemanager-service
    ${QtUseModules}
    ${PkgUserModules}
    )


