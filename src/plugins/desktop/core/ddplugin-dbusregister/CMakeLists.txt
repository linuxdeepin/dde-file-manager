cmake_minimum_required(VERSION 3.10)

project(ddplugin-dbusregister)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB DBUSREGISTER
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${DBUSREGISTER}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

# install dbus
install(FILES data/org.freedesktop.FileManager.service DESTINATION share/dbus-1/services)
