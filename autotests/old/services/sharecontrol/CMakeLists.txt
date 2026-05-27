cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test (sharecontrol needs DBus adaptor)
dfm_create_service_test_enhanced("sharecontrol" "${DFM_SOURCE_DIR}/services/sharecontrol") 