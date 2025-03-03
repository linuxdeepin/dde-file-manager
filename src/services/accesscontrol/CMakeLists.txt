cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-accesscontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AccessControl_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.AccessControl.xml)

find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS DBus REQUIRED)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )

# qt6_generate_dbus_interface(
#    accesscontroldbus.h
#    ${AccessControl_XML}
#    OPTIONS -M -S
# )

qt6_add_dbus_adaptor(FILEOPERATIONS_FILES ${AccessControl_XML}
    accesscontroldbus.h AccessControlDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)


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
pkg_check_modules(PolkitQt6
    REQUIRED
    polkit-qt6-1
    IMPORTED_TARGET
)

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    PkgConfig::crypt
    PkgConfig::PolkitAgent
    PkgConfig::PolkitQt6
    Qt6::DBus
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.accesscontrol.conf DESTINATION share/dbus-1/system.d/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/org.deepin.filemanager.accesscontrol.policy DESTINATION ${PolicyDir})
