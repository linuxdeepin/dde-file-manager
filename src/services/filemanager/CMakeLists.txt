cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-filemanager)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

# window service sources
FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB WINDOW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/private/*.cpp"
)

FILE(GLOB SHORTCUT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.cpp"
)

FILE(GLOB VAULT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.cpp"
)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${WINDOW_FILES}
    ${SHORTCUT_FILES}
    ${VAULT_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)

add_library(DFMSERVICE::filemanager ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/window
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
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