cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-recent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/services/dfmservices-business/dfmservice-filemanager
    )

set(recent_cxx_cpp
    recentfileinfo.cpp
    recentdiriterator.cpp
    recenteventreceiver.cpp
    recentbrowseviewmodel.cpp
    recentbrowseview.cpp
    recentlog.cpp
    recentutil.cpp
    recent.cpp
    recent.json
    )

set(recent_cxx_h
    recentfileinfo.h
    recentdiriterator.h
    recenteventreceiver.h
    recentbrowseviewmodel.h
    recentbrowseview.h
    recentlog.h
    recentutil.h
    recent.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${recent_cxx_h}
    ${recent_cxx_cpp}
    )

target_link_libraries(${PROJECT_NAME}
    dfm-base
    dfm-framework
    dfmservice-filemanager
    ${QtUseModules}
    ${PkgUserModules}
    )
