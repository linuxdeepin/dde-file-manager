cmake_minimum_required(VERSION 3.10)

project(dfm-base)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR
    ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386" OR
    ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    message(${CMAKE_SYSTEM_PROCESSOR} ": using deepin-anything.")
    ADD_DEFINITIONS(-DENABLE_QUICK_SEARCH)
endif()

# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")
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
             dfm_desktop_defines.h
             dfm_menu_defines.h
             )

FILE (GLOB INTERFACES_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/screen/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/screen/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/background/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/background/*.cpp"
)

FILE (GLOB UTILS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/decorator/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/decorator/*.cpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/configs/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/application/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/device/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/device/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/device/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/device/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/db/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/db/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/db/*.cpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/file/creator/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/file/creator/*.cpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmstatusbar/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmstatusbar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmstatusbar/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/dfmstatusbar/private/*.cpp"
)

FILE (GLOB DIALOGS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/mountpasswddialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/mountpasswddialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/settingsdialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/settingsdialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/settingsdialog/controls/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/settingsdialog/controls/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/taskdialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/taskdialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/basedialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/basedialog/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/smbsharepasswddialog/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/smbsharepasswddialog/*.cpp"
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
find_package(Qt5 COMPONENTS Sql REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5Codecs REQUIRED)

pkg_search_module(dfm-burn
    REQUIRED
    dfm-burn
    IMPORTED_TARGET
)

pkg_search_module(dfm-io
    REQUIRED
    dfm-io
    IMPORTED_TARGET
)

pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)

pkg_search_module(gsettings
    REQUIRED
    gsettings-qt
    IMPORTED_TARGET
)

# for generating middle source files of SettingsTemplate to translate.
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
set(TRANS_OF_SETTINGS_CPP)
set (DTK_SETTINGS_TOOLS_EXECUTABLE ${DTKCORE_TOOL_DIR}/dtk-settings)
if (EXISTS ${DTK_SETTINGS_TOOLS_EXECUTABLE})
    FILE (GLOB SETTING_TEMPALTE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/*.js")

    foreach(temp ${SETTING_TEMPALTE_FILES})
#        message("---- found template: ${temp}")
        string(REPLACE ".js" "-trans.cpp" TARGET_CPP ${temp})
#        message("---- convert to ${TARGET_CPP}")
        execute_process(COMMAND ${DTK_SETTINGS_TOOLS_EXECUTABLE} ${temp} -o ${TARGET_CPP})

        string(REPLACE "/" ";" PATH_FRAGS ${TARGET_CPP})
        list(GET PATH_FRAGS -1 FUNC_NAME)
        string(REPLACE ".cpp" "" FUNC_NAME ${FUNC_NAME})
        string(REPLACE "-" "_" FUNC_NAME ${FUNC_NAME})
        execute_process(COMMAND sed -i "5s/GenerateSettingTranslate/${FUNC_NAME}/" ${TARGET_CPP})

        list(APPEND TRANS_OF_SETTINGS_CPP ${TARGET_CPP})
    endforeach()

#    message(">>>> cpp source files for translating are generated: ${TRANS_OF_SETTINGS_CPP}")
    list(APPEND TRANS_OF_SETTINGS_CPP
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/global-setting-template-manully-trans.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/global-setting-template-dfmio-trans.cpp)
endif()

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
    ${DIALOGS_FILES}
    ${DBUSSERVER_FILES}
    ${SHORTCUT_FILES}
    ${TRANS_OF_SETTINGS_CPP}
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries( ${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::DBus
    Qt5::Sql
    Qt5::X11Extras
    PkgConfig::dfm-burn
    PkgConfig::dfm-io
    PkgConfig::dfm-mount
    PkgConfig::gsettings
    poppler-cpp
    KF5::Codecs
    ${DtkWidget_LIBRARIES}
)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE APPSHAREDIR="/usr/share/dde-file-manager"
)


add_library(DFM::base ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
        dbusservice
        ${DtkWidget_INCLUDEDIRS}
        KF5/kCodecs
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

set(AssetsPath ../../assets)
set(ShareDir ${CMAKE_INSTALL_PREFIX}/share/dde-file-manager)

FILE(GLOB SCHEMA_FILES ${AssetsPath}/gschema/*)
install(FILES ${SCHEMA_FILES} DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")

set(Mimetypes "${ShareDir}/mimetypes")
FILE(GLOB MIMETYPE_FILES ${AssetsPath}/mimetypes/*)
install(FILES ${MIMETYPE_FILES} DESTINATION ${Mimetypes})

set(MimetypeAssociations "${ShareDir}/mimetypeassociations")
FILE(GLOB MIMETYPEASSOCIATIONS_FILES ${AssetsPath}/mimetypeassociations/*)
install(FILES ${MIMETYPEASSOCIATIONS_FILES} DESTINATION ${MimetypeAssociations})

set(Templates "${ShareDir}/templates")
FILE(GLOB TEMPLATES_FILES ${AssetsPath}/templates/*)
install(FILES ${TEMPLATES_FILES} DESTINATION ${Templates})

set(OemMenuDir "${CMAKE_INSTALL_PREFIX}/share/deepin/dde-file-manager/oem-menuextensions")
install(FILES ${AssetsPath}/.readme DESTINATION ${OemMenuDir})

set(ContexMenuDir "${CMAKE_INSTALL_PREFIX}/share/applications/context-menus")
install(FILES ${AssetsPath}/.readme DESTINATION ${ContexMenuDir})

include(${CMAKE_SOURCE_DIR}/install_dconfig.cmake)
INSTALL_DCONFIG("org.deepin.dde.file-manager.json")
