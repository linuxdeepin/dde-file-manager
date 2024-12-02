cmake_minimum_required(VERSION 3.10)

project(dfmplugin-vault)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE VAULT_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(openssl REQUIRED libcrypto)
pkg_check_modules(secret REQUIRED libsecret-1 IMPORTED_TARGET)
pkg_check_modules(polkit REQUIRED polkit-agent-1 polkit-qt5-1)
set(VAULT_EXTRA_LIBRARIES ${polkit_LIBRARIES})

add_library(${PROJECT_NAME}
    SHARED
    ${VAULT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

find_package(Qt5 COMPONENTS Core)
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${VAULT_EXTRA_LIBRARIES}
    ${openssl_LIBRARIES}
    PkgConfig::secret
)

install(FILES polkit/com.deepin.filemanager.vault.policy
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.vault.json")
