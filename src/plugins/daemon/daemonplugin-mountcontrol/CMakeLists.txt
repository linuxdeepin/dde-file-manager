cmake_minimum_required(VERSION 3.10)

project(daemonplugin-mountcontrol)

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

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    Qt5::DBus
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt5_LIBRARIES}
    PkgConfig::mount
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DAEMON_EDGE_DIR}
)

#set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
#install(FILES polkit/com.deepin.filemanager.daemon.sharecontrol.policy DESTINATION ${PolicyDir})

#execute_process(COMMAND qdbuscpp2xml sharecontroldbus.h -o ./sharecontroldbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#execute_process(COMMAND qdbusxml2cpp -i ../sharecontroldbus.h -c ShareControlAdapter -l ShareControlDBus -a dbusadapter/sharecontrol_adapter sharecontroldbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
