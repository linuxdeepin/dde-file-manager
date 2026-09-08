cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create plugin test
# (sources are globbed at configure time: touch this file when adding tests)
dfm_create_plugin_test_enhanced("dfmplugin-tag" "${DFM_SOURCE_DIR}/plugins/common/dfmplugin-tag")