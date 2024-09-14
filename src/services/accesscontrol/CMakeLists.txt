cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-accesscontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(PkgConfig REQUIRED)

pkg_search_module(dfm-io
    REQUIRED
    dfm-io
    IMPORTED_TARGET
)
pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)

pkg_search_module(crypt
    REQUIRED
    libcryptsetup
    IMPORTED_TARGET
)
pkg_check_modules(PolkitAgent
    REQUIRED
    polkit-agent-1
    IMPORTED_TARGET
)
pkg_check_modules(PolkitQt5
    REQUIRED
    polkit-qt5-1
    IMPORTED_TARGET
)
find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    PkgConfig::dfm-io
    PkgConfig::dfm-mount
    PkgConfig::crypt
    PkgConfig::PolkitAgent
    PkgConfig::PolkitQt5
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.accesscontrol.conf DESTINATION share/dbus-1/system.d/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/org.deepin.filemanager.accesscontrol.policy DESTINATION ${PolicyDir})
