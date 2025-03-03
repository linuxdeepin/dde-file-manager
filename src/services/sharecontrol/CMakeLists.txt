cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-sharecontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ShareControl_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.ShareControl.xml)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )

find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS DBus REQUIRED)
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1)

# qt6_generate_dbus_interface(
#    sharecontroldbus.h
#    ${ShareControl_XML}
#    OPTIONS -M -S
# )

qt6_add_dbus_adaptor(FILEOPERATIONS_FILES ${ShareControl_XML}
    sharecontroldbus.h ShareControlDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    Qt6::DBus
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt6_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.usersharemanager.conf DESTINATION share/dbus-1/system.d/)
INSTALL_DBUS_SERVICE(org.deepin.Filemanager.UserShareManager root)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/org.deepin.filemanager.usersharemanager.policy DESTINATION ${PolicyDir})

#execute_process(COMMAND qdbuscpp2xml sharecontroldbus.h -o ./sharecontroldbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#execute_process(COMMAND qdbusxml2cpp -i ../sharecontroldbus.h -c ShareControlAdapter -l ShareControlDBus -a dbusadapter/sharecontrol_adapter sharecontroldbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
