cmake_minimum_required(VERSION 3.10)

project(dfmplugin-sidebar)
set(BIN_NAME dfm-sidebar-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include shared dependencies configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

# 创建共享库 - 先创建空的库，然后通过dependencies.cmake添加源文件
add_library(${BIN_NAME} SHARED)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Apply sidebar plugin specific configuration
dfm_setup_sidebar_dependencies(${BIN_NAME})

# Setup sidebar source files
dfm_setup_sidebar_sources(${BIN_NAME})

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.sidebar.json")

