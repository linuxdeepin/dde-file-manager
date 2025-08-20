cmake_minimum_required(VERSION 3.10)

# Use DFM test utilities to create plugin test
dfm_create_plugin_test("ddplugin-canvas" "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-canvas")

# Add desktoputils header files
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/ddplugin_eventinterface_helper.h
)

# Include desktoputils headers in test sources
target_sources(test-ddplugin-canvas PRIVATE ${EXT_FILES})

# Add plugins/desktop to include path
target_include_directories(test-ddplugin-canvas PRIVATE "${DFM_SOURCE_DIR}/plugins/desktop")
