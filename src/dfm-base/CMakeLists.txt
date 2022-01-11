cmake_minimum_required(VERSION 3.10)

project(dfm-base)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

# qrc
set(QRC_FILE
    qrc/skin/skin.qrc
    qrc/skin/filemanager.qrc
    qrc/themes/themes.qrc
    qrc/configure.qrc
    qrc/resources/resources.qrc
    qrc/chinese2pinyin/chinese2pinyin.qrc
    )
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

set(GLOBAL_H dfm_base_global.h
             dfm_event_defines.h
             dfm_global_defines.h
             dfm_actiontype_defines.h)

FILE (GLOB INTERFACES_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/private/*.cpp"
)

FILE (GLOB UTILS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"    
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/private/*.cpp"
)

FILE (GLOB MIMETYPE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/mimetype/*"
)

FILE (GLOB BASE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/base/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/private/*.cpp"
)

FILE (GLOB ALL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/file/local/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/local/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/local/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/local/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/entry/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/fileAction/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/fileAction/*.cpp"
)

FILE (GLOB WIDGETS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmfileview/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmfileview/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmfileview/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmfileview/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmkeyvaluelabel/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmkeyvaluelabel/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmsplitter/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmsplitter/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmwindow/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmwindow/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmwindow/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmwindow/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/action/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/action/*.cpp"
)

FILE (GLOB DBUSSERVER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/dbus_interface/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/dbus_interface/*.cpp"
)

FILE(GLOB SHORTCUT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.cpp"
)

set(DBUSSERVICE_H
    dbusservice/dbus_interface/devicemanagerdbus_interface.h
    dbusservice/global_server_defines.h
)

set(DBUSSERVICE_CPP
    dbusservice/dbus_interface/devicemanagerdbus_interface.cpp
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(util-dfm
    REQUIRED
    dfm-io dfm-mount
    IMPORTED_TARGET
)

add_library(${PROJECT_NAME}
    SHARED
    ${QRC_RESOURCES}
    ${GLOBAL_H}
    ${INTERFACES_FILES}
    ${UTILS_FILES}
    ${MIMETYPE_FILES}
    ${BASE_FILES}
    ${ALL_FILES}
    ${WIDGETS_FILES}
    ${DBUSSERVER_FILES}
    ${SHORTCUT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries( ${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::DBus
    Qt5::X11Extras
    PkgConfig::util-dfm
    ${DtkWidget_LIBRARIES}
)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE APPSHAREDIR="/usr/share/dde-file-manager"
)


add_library(DFM::base ALIAS ${PROJECT_NAME})


target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}
        dbusservice
        ${DtkWidget_INCLUDEDIRS}
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
    PATTERN "resources" EXCLUDE  
)

# schemas
FILE(GLOB SCHEMA_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/gschema/*.xml"
)

install(FILES ${SCHEMA_FILES} DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")
