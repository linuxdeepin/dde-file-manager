cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-opticalshare)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../DFMServiceCommon.cmake)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

dfm_setup_opticalshare_dependencies(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.filemanager.opticalshare.conf DESTINATION share/dbus-1/system.d/)

INSTALL_DBUS_SERVICE(org.deepin.Filemanager.OpticalShare root)
