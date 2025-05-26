cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-mountcontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(MountControl_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.MountControl.xml)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.policy"
    )

find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS Core DBus REQUIRED)
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1)
pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)

# qt6_generate_dbus_interface(
#    mountcontroldbus.h
#    ${MountControl_XML}
#    OPTIONS -M -S
# )

qt6_add_dbus_adaptor(FILEOPERATIONS_FILES ${MountControl_XML}
    mountcontroldbus.h MountControlDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::DBus
    DFM6::base
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt6_LIBRARIES}
    PkgConfig::mount
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.mountcontrol.conf DESTINATION share/dbus-1/system.d/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES org.deepin.filemanager.mountcontrol.policy DESTINATION ${PolicyDir})

INSTALL_DBUS_SERVICE(org.deepin.Filemanager.MountControl root)
