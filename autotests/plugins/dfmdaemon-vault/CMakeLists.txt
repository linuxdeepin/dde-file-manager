cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create plugin test
dfm_create_plugin_test_enhanced("dfmdaemon-vault" "${DFM_SOURCE_DIR}/plugins/daemon/vault") 