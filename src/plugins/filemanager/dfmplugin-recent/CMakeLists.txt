cmake_minimum_required(VERSION 3.0.2)

project(dfmplugin-recent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(recent_cxx_cpp
    recentfileinfo.cpp
    recentdiriterator.cpp
    recenteventreceiver.cpp
    recentbrowseviewmodel.cpp
    recentbrowseview.cpp
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
    recentutil.h
    recent.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${recent_cxx_h}
    ${recent_cxx_cpp}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    dfmservice-common
    dfmservice-filemanager
    ${QtUseModules}
    ${PkgUserModules}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
    )
