cmake_minimum_required(VERSION 3.10)

# Use DFM default test utilities to create plugin test
dfm_create_plugin_test("dfmplugin-encrypt-manager" "${DFM_SOURCE_DIR}/plugins/filemanager/dfmplugin-encrypt-manager")
