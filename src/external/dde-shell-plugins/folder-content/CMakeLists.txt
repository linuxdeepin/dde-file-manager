# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
if(NOT TARGET Dde::Shell)
    find_package(DDEShell REQUIRED)
endif()
# The plugin needs the dde-shell 2.0.61 dock API (dock/foldercontent.h,
# Dde::ShellDock). Skip gracefully on older dde-shell versions instead of
# failing the whole build.
find_package(DDEShellDock 2.0.61 QUIET)
if(NOT TARGET Dde::ShellDock)
    message(STATUS "dde-shell >= 2.0.61 not found, skipping folder-content plugin")
    return()
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Concurrent Network)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)

# The shell only loads translations for applet plugins (appletloader.cpp
# reads /usr/share/dde-shell/<id>/translations). Folder content providers are
# plain QPluginLoader modules without that hook, so compile the catalogs into
# the plugin's own resources and install a QTranslator on load instead.
# Catalog names follow the project-wide locale list (see
# translations/disk-encrypt-plugin). Languages are picked up automatically:
# drop a dfm-folder-content_<locale>.ts here and it gets compiled in.
file(GLOB FOLDERCONTENT_TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-folder-content*.ts")
list(REMOVE_ITEM FOLDERCONTENT_TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-folder-content.ts")
set(FOLDERCONTENT_QM_FILES "")
foreach(ts_file ${FOLDERCONTENT_TS_FILES})
    get_filename_component(catalog "${ts_file}" NAME_WE)
    set(qm_file "${CMAKE_CURRENT_BINARY_DIR}/translations/${catalog}.qm")
    list(APPEND FOLDERCONTENT_QM_FILES "${qm_file}")
    add_custom_command(
        OUTPUT "${qm_file}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/translations"
        COMMAND Qt6::lrelease "${ts_file}" -qm "${qm_file}"
        DEPENDS "${ts_file}"
        COMMENT "Compiling translation catalog ${ts_file}"
        VERBATIM)
endforeach()

set(FOLDERCONTENT_QRC "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")
set(FOLDERCONTENT_QRC_CONTENT "<RCC>\n  <qresource prefix=\"/dfm-folder-content\">\n")
foreach(qm_file ${FOLDERCONTENT_QM_FILES})
    get_filename_component(qm_name "${qm_file}" NAME)
    string(APPEND FOLDERCONTENT_QRC_CONTENT
        "    <file alias=\"translations/${qm_name}\">translations/${qm_name}</file>\n")
endforeach()
string(APPEND FOLDERCONTENT_QRC_CONTENT "  </qresource>\n</RCC>\n")
file(GENERATE OUTPUT "${FOLDERCONTENT_QRC}" CONTENT "${FOLDERCONTENT_QRC_CONTENT}")

# AUTORCC cannot depend on generated .qm files (its helper target races with
# lrelease on a clean build), so run rcc explicitly with a full dependency
# chain instead of adding the qrc to the target sources.
set(FOLDERCONTENT_QRC_CPP "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
add_custom_command(
    OUTPUT "${FOLDERCONTENT_QRC_CPP}"
    COMMAND Qt6::rcc -name translations -o "${FOLDERCONTENT_QRC_CPP}" "${FOLDERCONTENT_QRC}"
    DEPENDS "${FOLDERCONTENT_QRC}" ${FOLDERCONTENT_QM_FILES}
    COMMENT "Embedding translation catalogs into dfm-folder-content"
    VERBATIM)

# A content provider, not a dock Applet. No shell applet metadata or default pin.
add_library(dfm-folder-content MODULE
    directorymodel.cpp directorymodel.h
    folderpresentation.cpp folderpresentation.h
    foldercontent.qrc
    "${FOLDERCONTENT_QRC_CPP}"
)
set_source_files_properties("${FOLDERCONTENT_QRC_CPP}" PROPERTIES SKIP_AUTOGEN ON)
set_target_properties(dfm-folder-content PROPERTIES
    AUTOMOC ON AUTORCC ON PREFIX ""
    OUTPUT_NAME "org.deepin.ds.folderprovider.filemanager"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
# Dtk6::Core provides DUtil (AM1 appId/object-path helpers for app launches);
# gio-unix-2.0 backs the GDesktopAppInfo launch fallback. Qt6::Network backs
# the NetworkGuard TCP probe against unreachable network mounts. Dde::ShellDock
# supplies the dock::FolderPresentation/Provider symbols (libdde-shell-dock),
# required since dde-shell 2.0.61 moved them out of libdde-shell.
target_link_libraries(dfm-folder-content PRIVATE Dde::ShellDock Qt6::Quick Qt6::Concurrent Qt6::Network Dtk6::Gui Dtk6::Core PkgConfig::GIO_UNIX)

include(GNUInstallDirs)
install(TARGETS dfm-folder-content LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/dde-shell")
