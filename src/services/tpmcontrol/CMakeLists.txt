cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-tpmcontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include service configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../DFMServiceCommon.cmake)

FILE(GLOB TPMCONTROL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${TPMCONTROL_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

# Apply service configuration using shared dependencies
dfm_setup_tpmcontrol_dependencies(${PROJECT_NAME} "tpmcontrol")

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.tpmcontrol.conf DESTINATION share/dbus-1/system.d/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/org.deepin.filemanager.tpmcontrol.policy DESTINATION ${PolicyDir})

INSTALL_DBUS_SERVICE(org.deepin.Filemanager.TPMControl root)
