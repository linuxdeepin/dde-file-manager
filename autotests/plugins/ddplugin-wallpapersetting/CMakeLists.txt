cmake_minimum_required(VERSION 3.10)

# Align with ddplugin-background test style
dfm_create_plugin_test("ddplugin-wallpapersetting" "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-wallpapersetting")

# Extra headers required from desktoputils used by sources/tests
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/ddplugin_eventinterface_helper.h
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
)

# Add extra headers to target
target_sources(test-ddplugin-wallpapersetting PRIVATE ${EXT_FILES})

# Add plugins/desktop include path
target_include_directories(test-ddplugin-wallpapersetting PRIVATE "${DFM_SOURCE_DIR}/plugins/desktop")