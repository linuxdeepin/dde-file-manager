cmake_minimum_required(VERSION 3.10)

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


FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/copyfiles/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/copyfiles/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/cutfiles/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/cutfiles/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/deletefiles/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/deletefiles/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/cleantrash/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/cleantrash/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/trashfiles/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/trashfiles/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/fileoperationutils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileoperations/fileoperationutils/*.cpp"
)

FILE(GLOB DIALOG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/taskdialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/taskdialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/propertydialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/propertydialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/mountpasswddialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/mountpasswddialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/settingsdialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialog/settingsdialog/*.cpp"
)

FILE(GLOB PROPERTYDIALOG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/propertydialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/propertydialog/*.cpp"
)

FILE(GLOB MENU_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/menu/*/*.cpp"
)

FILE(GLOB BLUETOOTH_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/bluetooth/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bluetooth/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bluetooth/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bluetooth/private/*.cpp"
)

FILE(GLOB DELEGATE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/delegate/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/delegate/*.cpp"
)

FILE(GLOB USERSHARE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/usershare/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/usershare/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/usershare/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/usershare/private/*.cpp"
)

FILE(GLOB BURN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/burn/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/burn/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${FILEOPERATIONS_FILES}
    ${DIALOG_FILES}
    ${MENU_FILES}
    ${PROPERTYDIALOG_FILES}
    ${BLUETOOTH_FILES}
    ${DELEGATE_FILES}
    ${USERSHARE_FILES}
    ${BURN_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Svg REQUIRED)


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
    Qt5::DBus
    Qt5::Svg
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
