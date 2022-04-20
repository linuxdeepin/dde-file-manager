cmake_minimum_required(VERSION 3.10)

project(daemonplugin-anything)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(AnythingServer REQUIRED deepin-anything-backend)


add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    Qt5::DBus
    ${AnythingServer_LIBRARIES}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${AnythingServer_INCLUDE_DIRS}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
