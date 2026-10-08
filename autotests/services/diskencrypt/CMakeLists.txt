cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-diskencrypt" "${DFM_SOURCE_DIR}/src/services/diskencrypt")

# ---------------------------------------------------------------------------
# PMS sev-2 regression tests: the diskencrypt service is built as an executable
# (deepin-diskencrypt-service), so its sources are unity-included by the test
# file instead of being linked. Provide the same dependency set the real
# service target uses (see src/services/diskencrypt/dependencies.cmake).
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(UT_DISKENC REQUIRED IMPORTED_TARGET libcryptsetup devmapper)
pkg_check_modules(UT_POLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)
find_package(deepin-qdbus-service REQUIRED)

target_include_directories(ut-dfmdaemon-diskencrypt PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt
)

target_compile_definitions(ut-dfmdaemon-diskencrypt PRIVATE
    SERVICE_CONFIG_DIR="/usr/share/deepin-service-manager/"
)

# 服务源码 unity-include 进测试 TU 后，把服务的 Q_OBJECT 头加入目标源列表，
# 由 AUTOMOC 为其生成元对象代码。
set(UT_DISKENC_MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/dbus/diskencryptsetup.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/dbus/diskencryptsetup_p.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/helpers/notificationhelper.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/helpers/overlaydmnotifyhelper.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/helpers/cryptsetupcompabilityhelper.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/workers/baseencryptworker.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/workers/dminitencryptworker.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/workers/normalinitencryptworker.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/workers/fstabinitencryptworker.h
    ${CMAKE_SOURCE_DIR}/src/services/diskencrypt/workers/resumeencryptworker.h
)
target_sources(ut-dfmdaemon-diskencrypt PRIVATE ${UT_DISKENC_MOC_HEADERS})

target_link_libraries(ut-dfmdaemon-diskencrypt PRIVATE
    Qt6::Concurrent
    deepin-qdbus-service
    PkgConfig::UT_POLKIT
    PkgConfig::UT_DISKENC
)
