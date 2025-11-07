cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-textindex)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include service configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

set(FULL_TEXT_PATH "${CMAKE_SOURCE_DIR}/3rdparty/fulltext")

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.policy"
    "${FULL_TEXT_PATH}/*.cpp"
    "${FULL_TEXT_PATH}/*.h"
    "${CMAKE_SOURCE_DIR}/include/dfm-base/dfm_log_defines.h"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

# Apply service configuration using shared dependencies
dfm_setup_textindex_dependencies(${PROJECT_NAME} "textindex")

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/user/)
INSTALL_DBUS_SERVICE(org.deepin.Filemanager.TextIndex)
INSTALL_DCONFIG("org.deepin.dde.file-manager.textindex.json")

# Install systemd drop-in configuration for resource limits
# This prevents OOM issues without modifying the shared service template
install(FILES
    systemd/memory-limit.conf
    DESTINATION
    lib/systemd/user/deepin-service-plugin@org.deepin.Filemanager.TextIndex.service.d/
)
