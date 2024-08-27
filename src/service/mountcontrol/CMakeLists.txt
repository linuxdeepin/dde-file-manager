cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-mountcontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
pkg_check_modules(PolkitQt5 REQUIRED polkit-qt5-1)
pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::DBus
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt5_LIBRARIES}
    PkgConfig::mount
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.mountcontrol.conf DESTINATION share/dbus-1/system.d/)
INSTALL_DBUS_SERVICE(org.deepin.Filemanager.MountControl root)
