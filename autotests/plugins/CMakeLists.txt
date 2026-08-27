# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# Plugin unit tests — covers Mid-priority methods across all plugin modules.
# Links against pre-built plugin shared libraries (same CMake project).

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# All plugin source directories for header resolution
set(_plugin_src ${DFM_SOURCE_DIR}/plugins)

# Additional plugin source files needed for unexported symbols
set(_extra_sources
    ${_plugin_src}/common/dfmplugin-tag/utils/tagtextformat.cpp
)

dfm_add_test(test-plugins
    SOURCES ${TEST_SOURCES} ${_extra_sources}
    LINK_LIBRARIES
        # filemanager plugins
        dfm-workspace-plugin
        dfm-titlebar-plugin
        dfm-computer-plugin
        dfm-vault-plugin
        dfm-sidebar-plugin
        dfm-trash-plugin
        dfm-optical-plugin
        dfm-smbbrowser-plugin
        dfm-recent-plugin
        dfm-avfsbrowser-plugin
        dfm-disk-encrypt-plugin
        dfm-myshares-plugin
        # common plugins
        dfm-tag-plugin
        dfm-burn-plugin
        dfm-fileoperations-plugin
        dfm-propertydialog-plugin
        dfm-menu-plugin
        dfm-bookmark-plugin
        dfm-utils-plugin
        dfm-dirshare-plugin
        # desktop plugins
        dd-organizer-plugin
        dd-canvas-plugin
        dd-core-plugin
        dd-wallpapersetting-plugin
        dd-background-plugin
        # daemon plugins
        dfmdaemon-tag-plugin
        dfmdaemon-recent-plugin
        # filedialog
        filedialog-core-plugin
        # core libs
        DFM6::base
        DFM6::framework
        Qt6::Core
        Qt6::Widgets
        Qt6::Test
        Dtk6::Widget
)

# Add all plugin directories to include path for header resolution
file(GLOB _plugin_dirs LIST_DIRECTORIES true "${_plugin_src}/*/*")
foreach(_d ${_plugin_dirs})
    if(IS_DIRECTORY ${_d})
        target_include_directories(test-plugins PRIVATE ${_d})
    endif()
endforeach()
# Also add top-level plugin subdirs
target_include_directories(test-plugins PRIVATE
    ${_plugin_src}/filemanager
    ${_plugin_src}/common
    ${_plugin_src}/desktop
    ${_plugin_src}/daemon
    ${_plugin_src}/filedialog
    ${DFM_SOURCE_DIR}
    ${DFM_SOURCE_DIR}/dfm-base
)
