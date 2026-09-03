cmake_minimum_required(VERSION 3.10)

# SPDX-License-Identifier: GPL-3.0-or-later
# dfmplugin-vault unit tests — recompile the vault plugin's own sources into the
# test binary so stub-ext can access private members. The vault plugin's
# dependencies.cmake (dfm_setup_vault_dependencies) is auto-applied via
# dfm_create_plugin_test -> dfm_configure_plugin_dependencies, which adds
# openssl / libsecret / polkit / libcryptsetup.

dfm_create_plugin_test("dfmplugin-vault" "${DFM_SOURCE_DIR}/plugins/filemanager/dfmplugin-vault")

# Vault sources use cross-plugin includes via the filemanager plugins root.
target_include_directories(ut-dfmplugin-vault PRIVATE
    "${DFM_SOURCE_DIR}/plugins/filemanager"
)
