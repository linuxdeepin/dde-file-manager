cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")

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

# find source code
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/dfm-base/*")
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.hpp"
    "./*.cpp"
)

# add qrc resources
set(QRC_FILES
    qrc/skin/skin.qrc
    qrc/skin/filemanager.qrc
    qrc/themes/themes.qrc
    qrc/configure.qrc
    qrc/resources/resources.qrc
    qrc/chinese2pinyin/chinese2pinyin.qrc
)

include(dfm-base-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfm-base-qt6.cmake)
endif()

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
install(PROGRAMS ${DFM_DLNFS_SCRIPT_LAUNCHER} DESTINATION bin)

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
INSTALL_DCONFIG("org.deepin.dde.file-manager.animation.json")

