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

find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${core_cxx_cpp}
    ${core_cxx_h}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    DFMSERVICE::filemanager
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
