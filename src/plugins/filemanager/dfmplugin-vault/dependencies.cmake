# dependencies.cmake - Dependencies configuration for dfmplugin-vault
# This file defines the specific dependencies and configuration for the vault plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup vault plugin dependencies
function(dfm_setup_vault_dependencies target_name)
    message(STATUS "DFM: Setting up vault plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    pkg_check_modules(openssl REQUIRED libcrypto)
    pkg_check_modules(secret REQUIRED libsecret-1 IMPORTED_TARGET)
    pkg_check_modules(polkit REQUIRED polkit-agent-1 polkit-qt6-1)
    pkg_check_modules(libcryptsetup REQUIRED libcryptsetup)

    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add vault-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        ${polkit_LIBRARIES}
        ${openssl_LIBRARIES}
        ${libcryptsetup_LIBRARIES}
        PkgConfig::secret
    )

    target_include_directories(${target_name}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
    )

    message(STATUS "DFM: Vault plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Vault plugin dependencies configuration loaded")
