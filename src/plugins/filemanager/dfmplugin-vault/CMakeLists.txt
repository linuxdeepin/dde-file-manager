cmake_minimum_required(VERSION 3.10)

project(dfmplugin-vault)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB VAULT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(polkit REQUIRED polkit-agent-1 polkit-qt5-1)
pkg_check_modules(openssl REQUIRED libcrypto)

add_library(${PROJECT_NAME}
    SHARED
    ${VAULT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${polkit_LIBRARIES}
    ${openssl_LIBRARIES}
)

install(FILES polkit/com.deepin.filemanager.vault.policy
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_EDGE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.vault.json")
