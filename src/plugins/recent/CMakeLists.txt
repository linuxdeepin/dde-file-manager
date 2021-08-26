cmake_minimum_required(VERSION 3.0.2)

project(recent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(recent_cxx_source
    dfmbookmarknode.cpp
    dfmrecentutil.cpp
    dfmrecentfileinfo.cpp
    recent.cpp
    recent.json
    )

set(recent_cxx_include
    dfmbookmarknode.h
    dfmrecentutil.h
    dfmrecentfileinfo.h
    recent.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${recent_cxx_source}
    ${recent_cxx_include}
    )

target_link_libraries(${PROJECT_NAME}
    dfmbundleservice
    dfmlifecycle
    ${QtUseModules}
    ${PkgUserModules}
    )
