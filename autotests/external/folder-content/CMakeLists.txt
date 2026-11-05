# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Unit tests for the folder-content dock plugin
# (src/external/dde-shell-plugins/folder-content). The plugin sources are
# compiled directly into the test binary, like the disk-mount dock tests.

# The dock API (dock/foldercontent.h, libdde-shell-dock) exists since
# dde-shell 2.0.61; skip on older shells instead of failing the test build.
find_package(DDEShellDock 2.0.61 QUIET)
if(NOT TARGET Dde::ShellDock)
    message(STATUS "dde-shell >= 2.0.61 not found, skipping folder-content unit tests")
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Concurrent Network Widgets Test)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UTFolderContent REQUIRED IMPORTED_TARGET gio-unix-2.0)

set(_plugin_dir "${DFM_SOURCE_DIR}/external/dde-shell-plugins/folder-content")

file(GLOB _ut_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

dfm_add_test(ut-folder-content
    SOURCES ${_ut_files}
            "${_plugin_dir}/directorymodel.cpp"
            "${_plugin_dir}/folderpresentation.cpp"
            "${_plugin_dir}/foldercontent.qrc"
    LINK_LIBRARIES
        Dde::ShellDock
        Qt6::Quick
        Qt6::Concurrent
        Qt6::Network
        Qt6::Widgets
        Qt6::Test
        Dtk6::Gui
        Dtk6::Core
        PkgConfig::UTFolderContent
)

# Q_OBJECT headers for AUTOMOC.
target_sources(ut-folder-content PRIVATE
    "${_plugin_dir}/directorymodel.h"
    "${_plugin_dir}/folderpresentation.h"
)
set_target_properties(ut-folder-content PROPERTIES AUTOMOC ON AUTORCC ON)
target_include_directories(ut-folder-content PRIVATE "${_plugin_dir}")

# The QML/icon tests need an offscreen windowing stack.
set_tests_properties(ut-folder-content PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
