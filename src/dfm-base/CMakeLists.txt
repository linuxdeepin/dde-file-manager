cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")

set(BIN_NAME dfm-base)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 1)
endif()

# Disbale qDebug(), qInfo(), qWaring(), qCritical()
# Use qCDebug() instead of qDebug()
if(OPT_DISABLE_QDEBUG)
    add_definitions(-DDFM_DISABLE_DEBUG_MACRO)
endif()

# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")

# add qrc resources
set(QRC_FILES
    qrc/skin/skin.qrc
    qrc/skin/filemanager.qrc
    qrc/themes/themes.qrc
    qrc/configure.qrc
    qrc/resources/resources.qrc
    qrc/chinese2pinyin/chinese2pinyin.qrc
    )

if(DFM_BUILD_WITH_QT6)
    qt_add_resources(QRC_RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
endif()

# add code
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/*")
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.hpp"
    "./*.cpp"
)

# add depends
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Concurrent DBus Sql Network REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(dfm-burn REQUIRED dfm${DTK_VERSION_MAJOR}-burn IMPORTED_TARGET)
pkg_search_module(dfm-io REQUIRED dfm${DTK_VERSION_MAJOR}-io IMPORTED_TARGET)
pkg_search_module(dfm-mount REQUIRED dfm${DTK_VERSION_MAJOR}-mount IMPORTED_TARGET)
pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)

if(OPT_ENABLE_QML)
    set(DFM_EXTRA_LIBRARIES "")
else()
    find_package(KF5Codecs REQUIRED)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
    pkg_search_module(gsettings REQUIRED gsettings-qt IMPORTED_TARGET)
    pkg_search_module(X11 REQUIRED x11 IMPORTED_TARGET)
    set(DFM_EXTRA_LIBRARIES
        Qt${QT_VERSION_MAJOR}::X11Extras
        PkgConfig::gsettings
        PkgConfig::X11
        poppler-cpp
        KF5::Codecs)
endif()


# generate dbus interface
if(DFM_BUILD_WITH_QT6)
    qt_add_dbus_interface(SRCS
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.DeviceManager.xml
        devicemanager_interface)
else()
    qt5_add_dbus_interface(SRCS
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.DeviceManager.xml
        devicemanager_interface)
endif()

# for generating middle source files of SettingsTemplate to translate.
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

# build
add_library(${BIN_NAME}
    SHARED
    ${QRC_RESOURCES}
    ${INCLUDE_FILES}
    ${SRCS}
)

target_link_libraries(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    PkgConfig::dfm-burn
    PkgConfig::dfm-mount
    PkgConfig::dfm-io
    PkgConfig::mount
    ${DFM_EXTRA_LIBRARIES}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

set(ShareDir ${CMAKE_INSTALL_PREFIX}/share/dde-file-manager) # also use for install
target_compile_definitions(
        ${BIN_NAME}
        PRIVATE APPSHAREDIR="${ShareDir}"
        PRIVATE DFM_BASE_INTERNAL_USE=1
)

add_library(DFM::base ALIAS ${BIN_NAME})

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# install library file
install(TARGETS ${BIN_NAME} LIBRARY DESTINATION  ${LIB_INSTALL_DIR})
# install header files
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# install resources files
set(AssetsPath ${PROJECT_SOURCE_DIR}/assets)

FILE(GLOB SCHEMA_FILES ${AssetsPath}/gschema/*)
install(FILES ${SCHEMA_FILES} DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")

set(DLNFS_SCRIPT ${AssetsPath}/scripts/dfm-dlnfs-automount)
install(FILES ${DLNFS_SCRIPT} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/deepin/dde-file-manager)

set(DLNFS_SCRIPT_LAUNCHER ${AssetsPath}/scripts/99dfm-dlnfs-automount)
install(FILES ${DLNFS_SCRIPT_LAUNCHER} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/X11/Xsession.d)

set(DFM_DLNFS_SCRIPT_LAUNCHER
    ${AssetsPath}/scripts/dde-file-manager
    ${AssetsPath}/scripts/file-manager.sh)
install(FILES ${DFM_DLNFS_SCRIPT_LAUNCHER} DESTINATION bin)

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

# log viewer
set(LogViewerConfDir "${CMAKE_INSTALL_PREFIX}/share/deepin-log-viewer/deepin-log.conf.d")
install(FILES ${AssetsPath}/log/viewer/dde-file-manager.json DESTINATION ${LogViewerConfDir})

# log debug
set(LogDebugConfDir "${CMAKE_INSTALL_PREFIX}/share/deepin-debug-config/deepin-debug-config.d")
install(FILES ${AssetsPath}/log/debug/org.deepin.file-manager.json DESTINATION ${LogDebugConfDir})

INSTALL_DCONFIG("org.deepin.dde.file-manager.json")
INSTALL_DCONFIG("org.deepin.dde.file-manager.plugins.json")
INSTALL_DCONFIG("org.deepin.dde.file-manager.view.json")

