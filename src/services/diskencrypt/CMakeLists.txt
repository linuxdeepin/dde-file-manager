cmake_minimum_required(VERSION 3.0)

set(BIN_NAME "deepin-diskencrypt-service")
project(${BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DiskEncrypt_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.DiskEncrypt.xml)

find_package(Qt5 REQUIRED COMPONENTS Core Concurrent DBus)
find_package(deepin-qdbus-service REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CryptSetup REQUIRED libcryptsetup)
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1 IMPORTED_TARGET)
pkg_check_modules(PolkitQt5 REQUIRED polkit-qt5-1 IMPORTED_TARGET)

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")

file(GLOB_RECURSE SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# generate dbus xml and adaptor
# DBus: DiskEncryptDBus
qt5_generate_dbus_interface(
   diskencryptdbus.h
   ${DiskEncrypt_XML}
   OPTIONS -M -S
)
qt5_add_dbus_adaptor(SRCS
                     ${DiskEncrypt_XML}
                     diskencryptdbus.h
                     DiskEncryptDBus)


add_executable(${BIN_NAME}
    ${SRCS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core
    Qt5::Concurrent
    Qt5::DBus
    DFM::base
    PkgConfig::PolkitAgent
    PkgConfig::PolkitQt5
    ${CryptSetup_LIBRARIES}
    ${deepin-qdbus-service_LIBRARIES}
)

target_include_directories(${BIN_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CryptSetup_INCLUDE_DIRS}
    ${deepin-qdbus-service_INCLUDE_DIR}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.filemanager.diskencrypt.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.deepin.Filemanager.DiskEncrypt.service DESTINATION share/dbus-1/system-services/)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/policy/org.deepin.filemanager.diskencrypt.policy
    DESTINATION ${PolicyDir})
install(FILES polkit/rules/99-dde-file-manager-encrypt.pkla
    DESTINATION /etc/polkit-1/localauthority/10-vendor.d)
