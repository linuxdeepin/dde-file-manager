cmake_minimum_required(VERSION 3.10)

# SPDX-License-Identifier: GPL-3.0-or-later
# dfmplugin-disk-encrypt-entry unit tests — recompile the plugin's own sources.
# dependencies.cmake (dfm_setup_disk-encrypt-entry_dependencies) auto-applied.

dfm_create_plugin_test("dfmplugin-disk-encrypt-entry" "${DFM_SOURCE_DIR}/plugins/filemanager/dfmplugin-disk-encrypt-entry")

target_include_directories(ut-dfmplugin-disk-encrypt-entry PRIVATE
    "${DFM_SOURCE_DIR}/plugins/filemanager"
)
