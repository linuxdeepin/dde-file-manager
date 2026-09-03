cmake_minimum_required(VERSION 3.10)

# SPDX-License-Identifier: GPL-3.0-or-later
# dfmplugin-detailspace unit tests — recompile the plugin's own sources.
# No custom dependencies.cmake -> default plugin config (DFM6::base/framework).

dfm_create_plugin_test("dfmplugin-detailspace" "${DFM_SOURCE_DIR}/plugins/filemanager/dfmplugin-detailspace")

target_include_directories(ut-dfmplugin-detailspace PRIVATE
    "${DFM_SOURCE_DIR}/plugins/filemanager"
)
