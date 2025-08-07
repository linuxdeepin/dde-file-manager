cmake_minimum_required(VERSION 3.10)

# Use DFM test utilities to create plugin test
dfm_create_plugin_test("ddplugin-core" "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-core")

# Remove conflicting DBus monitor files and related dependencies
get_target_property(CORE_SOURCES test-ddplugin-core SOURCES)
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/dbusmonitor1?\\.(h|cpp)$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/screenproxydbus\\.(h|cpp)$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/screendbus\\.(h|cpp)$")
set_target_properties(test-ddplugin-core PROPERTIES SOURCES "${CORE_SOURCES}")

# Add desktoputils header files
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/ddplugin_eventinterface_helper.h
)

# Include desktoputils headers in test sources
target_sources(test-ddplugin-core PRIVATE ${EXT_FILES})

# Add necessary include paths for compilation and moc
target_include_directories(test-ddplugin-core PRIVATE 
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${DFM_INCLUDE_DIR}"
    "${DFM_INCLUDE_DIR}/dfm-base"
)

# Configure automoc to include dfm-base headers
set_property(TARGET test-ddplugin-core PROPERTY AUTOMOC_MOC_OPTIONS "-I${DFM_INCLUDE_DIR}/dfm-base") 
