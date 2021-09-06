cmake_minimum_required(VERSION 3.0.2)

project(recent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/services/dfm-business-services/dfm-filemanager-service
    )

set(recent_cxx_source
    recenteventrecver.cpp
    recentbrowseviewmodel.cpp
    recentbrowseview.cpp
    recentlog.cpp
    recentutil.cpp
    recent.cpp
    recent.json
    )

set(recent_cxx_include
    recenteventrecver.h
    recentbrowseviewmodel.h
    recentbrowseview.h
    recentlog.h
    recentutil.h
    recent.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${recent_cxx_source}
    ${recent_cxx_include}
    )

target_link_libraries(${PROJECT_NAME}
    dfm-base
    dfm-framework
    dfm-filemanager-service
    ${QtUseModules}
    ${PkgUserModules}
    )
