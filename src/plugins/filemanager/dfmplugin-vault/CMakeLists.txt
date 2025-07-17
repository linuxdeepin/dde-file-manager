cmake_minimum_required(VERSION 3.10)

project(dfmplugin-vault)
set(BIN_NAME dfm-vault-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include DFM plugin configuration module
include(DFMPluginConfig)

FILE(GLOB_RECURSE VAULT_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${BIN_NAME}
    SHARED
    ${VAULT_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use DFM plugin configuration system
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
dfm_setup_vault_dependencies(${BIN_NAME})

install(FILES polkit/com.deepin.filemanager.vault.policy
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.vault.json")
