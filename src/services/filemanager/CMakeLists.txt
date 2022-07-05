cmake_minimum_required(VERSION 3.10)

project(dfmservice-filemanager)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

# windows service sources
FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB WORKSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.cpp"
)

FILE(GLOB DETAILSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*/*.cpp"
)


add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${WORKSPACE_FILES}
    ${DETAILSPACE_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)

add_library(DFMSERVICE::filemanager ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/windows
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    Qt5::X11Extras
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/dfmservice-filemanager
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
