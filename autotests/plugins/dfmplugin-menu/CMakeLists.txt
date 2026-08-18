cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create plugin test
dfm_create_plugin_test_enhanced("dfmplugin-menu" "${DFM_SOURCE_DIR}/plugins/common/dfmplugin-menu")

# Match production plugin build (src/plugins/common/dfmplugin-menu/CMakeLists.txt)
# so focus-only code paths (e.g. ExtendMenuScenePrivate::mayComboPostion) stay live.
target_compile_definitions(ut-dfmplugin-menu PRIVATE MENU_CHECK_FOCUSONLY)
