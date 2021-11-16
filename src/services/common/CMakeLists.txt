cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-common)

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

FILE(GLOB EVENT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.cpp"
    )

FILE(GLOB JOB_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/job/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/job/*.cpp"
    )

FILE(GLOB DEVICE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.cpp"
    )

FILE(GLOB DIALOG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.cpp"
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

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${QtUseModules}
    ${PkgUseModules}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
    )