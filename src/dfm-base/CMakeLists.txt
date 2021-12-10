cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

set(GLOBAL_H dfm_base_global.h)

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
    )

# TODO(zhangs): move `WIDGETS_*` to here
FILE (GLOB WIDGETS_FILES
    )

FILE (GLOB DBUSSERVER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/dbus_interface/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbusservice/dbus_interface/*.cpp"
    )


# TODO(zhangs): remove `WIDGETS_*`
set(WIDGETS_H
    widgets/dfmfileview/private/fileview_p.h
    widgets/dfmfileview/private/fileviewmodel_p.h
    widgets/dfmfileview/private/traversaldirthread.h
    widgets/dfmfileview/private/fileviewitem_p.h
    widgets/dfmfileview/private/delegatecommon.h
    widgets/dfmfileview/private/baseitemdelegate_p.h
    widgets/dfmfileview/private/iconitemdelegate_p.h
    widgets/dfmfileview/private/listitemdelegate_p.h
    widgets/dfmfileview/private/fileselectionmodel_p.h
    widgets/dfmsidebar/private/sidebar_p.h
    widgets/dfmsidebar/private/sidebarview_p.h
    widgets/dfmsidebar/private/sidebarmodel_p.h
    widgets/dfmsidebar/sidebar.h
    widgets/dfmsidebar/sidebaritem.h
    widgets/dfmsidebar/sidebaritemdelegate.h
    widgets/dfmsidebar/sidebarmodel.h
    widgets/dfmsidebar/sidebarview.h
    widgets/dfmsplitter/splitter.h
    widgets/dfmfileview/fileview.h
    widgets/dfmfileview/fileviewmodel.h
    widgets/dfmfileview/headerview.h
    widgets/dfmfileview/fileviewitem.h
    widgets/dfmfileview/baseitemdelegate.h
    widgets/dfmfileview/listitemdelegate.h
    widgets/dfmfileview/iconitemdelegate.h
    widgets/dfmfileview/itemdelegatehelper.h
    widgets/dfmfileview/filesortfilterproxymodel.h
    widgets/dfmfileview/fileselectionmodel.h
    widgets/dfmkeyvaluelabel/keyvaluelabel.h
    )

set(WIDGETS_CPP
    widgets/dfmfileview/private/traversaldirthread.cpp
    widgets/dfmfileview/private/fileview_p.cpp
    widgets/dfmfileview/private/delegatecommon.cpp
    widgets/dfmfileview/private/baseitemdelegate_p.cpp
    widgets/dfmfileview/private/iconitemdelegate_p.cpp
    widgets/dfmfileview/private/listitemdelegate_p.cpp
    widgets/dfmfileview/private/fileselectionmodel_p.cpp
    widgets/dfmsidebar/private/sidebar_p.cpp
    widgets/dfmsidebar/sidebar.cpp
    widgets/dfmsidebar/sidebaritem.cpp
    widgets/dfmsidebar/sidebaritemdelegate.cpp
    widgets/dfmsidebar/sidebarmodel.cpp
    widgets/dfmsidebar/sidebarview.cpp
    widgets/dfmsplitter/splitter.cpp
    widgets/dfmfileview/fileview.cpp
    widgets/dfmfileview/fileviewmodel.cpp
    widgets/dfmfileview/headerview.cpp
    widgets/dfmfileview/fileviewitem.cpp
    widgets/dfmfileview/baseitemdelegate.cpp
    widgets/dfmfileview/listitemdelegate.cpp
    widgets/dfmfileview/iconitemdelegate.cpp
    widgets/dfmfileview/itemdelegatehelper.cpp
    widgets/dfmfileview/filesortfilterproxymodel.cpp
    widgets/dfmfileview/fileselectionmodel.cpp
    widgets/dfmkeyvaluelabel/keyvaluelabel.cpp
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
find_package(PkgConfig REQUIRED)

pkg_search_module(util-dfm
    REQUIRED
    dfm-io dfm-mount
    IMPORTED_TARGET
)

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${INTERFACES_FILES}
    ${UTILS_FILES}
    ${MIMETYPE_FILES}
    ${BASE_FILES}
    ${ALL_FILES}
    ${WIDGETS_H}
    ${WIDGETS_CPP}
    ${DBUSSERVER_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries( ${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::DBus
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
