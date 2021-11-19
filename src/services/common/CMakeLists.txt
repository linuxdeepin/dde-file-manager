cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-common)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB EVENT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.cpp"
)

FILE(GLOB JOB_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/*.cpp"
)

FILE(GLOB DEVICE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.cpp"
)

FILE(GLOB DIALOG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/taskdialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/taskdialog/*.cpp"
)

FILE(GLOB MENU_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.cpp"
# todo:
#    "${CMAKE_CURRENT_SOURCE_DIR}/menu/menucore/private/*.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/menu/menucore/*.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/menu/menucore/*.cpp"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${EVENT_FILES}
    ${JOB_FILES}
    ${DEVICE_FILES}
    ${DIALOG_FILES}
    ${MENU_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(gsettings
    REQUIRED
    gsettings-qt
    IMPORTED_TARGET
)

pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)


add_library(DFMSERVICE::common ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/device
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog
    ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    PkgConfig::gsettings
    PkgConfig::dfm-mount
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
    DESTINATION include/dfmservice-common
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)