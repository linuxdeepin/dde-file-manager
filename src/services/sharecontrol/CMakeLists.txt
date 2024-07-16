cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-sharecontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
pkg_check_modules(PolkitQt5 REQUIRED polkit-qt5-1)


add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    Qt5::DBus
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt5_LIBRARIES}
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
