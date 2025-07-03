cmake_minimum_required(VERSION 3.10)

set(BIN_NAME "deepin-diskencrypt-service")
project(${BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DiskEncrypt_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.DiskEncrypt.xml)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent DBus)
find_package(deepin-qdbus-service REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CryptSetup REQUIRED libcryptsetup)
pkg_check_modules(DevMapper REQUIRED devmapper)
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1 IMPORTED_TARGET)
pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1 IMPORTED_TARGET)

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")

file(GLOB_RECURSE SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# generate dbus xml and adaptor
# DBus: DiskEncryptDBus
qt6_generate_dbus_interface(
   dbus/diskencryptsetup.h
   ${DiskEncrypt_XML}
   OPTIONS -M -S
)
qt6_add_dbus_adaptor(SRCS
                     ${DiskEncrypt_XML}
                     dbus/diskencryptsetup.h
                     DiskEncryptSetup)


add_executable(${BIN_NAME}
    ${SRCS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::DBus
    DFM6::base
    PkgConfig::PolkitAgent
    PkgConfig::PolkitQt6
    deepin-qdbus-service
    ${CryptSetup_LIBRARIES}
    ${DevMapper_LIBRARIES}
)

target_include_directories(${BIN_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CryptSetup_INCLUDE_DIRS}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.filemanager.diskencrypt.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.deepin.Filemanager.DiskEncrypt.service DESTINATION share/dbus-1/system-services/)
install(FILES ${CMAKE_SOURCE_DIR}/assets/rules/99-dfm-encrypt.rules DESTINATION /etc/udev/rules.d)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/policy/org.deepin.filemanager.diskencrypt.policy
    DESTINATION ${PolicyDir})
install(FILES polkit/rules/99-dde-file-manager-encrypt.pkla
    DESTINATION /etc/polkit-1/localauthority/10-vendor.d)
