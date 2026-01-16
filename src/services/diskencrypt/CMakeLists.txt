cmake_minimum_required(VERSION 3.10)

set(BIN_NAME "deepin-diskencrypt-service")
project(${BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include service configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

file(GLOB_RECURSE SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${BIN_NAME}
    ${SRCS}
)

# Apply service configuration using shared dependencies
dfm_setup_diskencrypt_dependencies(${PROJECT_NAME})

# Enable position-independent executables for improved security
target_link_options(${BIN_NAME} PRIVATE -pie)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.filemanager.diskencrypt.conf DESTINATION share/dbus-1/system.d/)
install(FILES deepin-filemanager-diskencrypt.service DESTINATION lib/systemd/system/)
install(FILES deepin-diskencrypt-tmpfiles.conf DESTINATION lib/tmpfiles.d/)
install(FILES org.deepin.Filemanager.DiskEncrypt.service DESTINATION share/dbus-1/system-services)
install(FILES ${CMAKE_SOURCE_DIR}/assets/rules/99-dfm-encrypt.rules DESTINATION /etc/udev/rules.d)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
set(RulesDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/rules.d")

install(FILES polkit/policy/org.deepin.filemanager.diskencrypt.policy
    DESTINATION ${PolicyDir})

# Install polkit rules (JavaScript format, replaces deprecated .pkla)
install(FILES polkit/rules/99-dde-file-manager-encrypt.rules
    DESTINATION ${RulesDir})
