cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-accesscontrol" "${DFM_SOURCE_DIR}/src/services/accesscontrol")

# ---------------------------------------------------------------------------
# accesscontrol service sources are unity-included by the test TU (the service
# has no library target to link against), so provide the same dependency set
# the real service uses (see src/services/accesscontrol/dependencies.cmake).
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus Concurrent)
find_package(Dtk6 COMPONENTS Core REQUIRED)
pkg_check_modules(UT_ACC_CRYPT REQUIRED IMPORTED_TARGET libcryptsetup)
pkg_check_modules(UT_ACC_POLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)

target_include_directories(ut-dfmdaemon-accesscontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/accesscontrol
    ${CMAKE_SOURCE_DIR}/src/services/common
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate the DBus adaptor the service source includes
# ("accesscontroladaptor.h", class AccessControlManagerAdaptor).
qt6_add_dbus_adaptor(UT_ACC_ADAPTOR_SRC
    "${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.AccessControl.xml"
    ${CMAKE_SOURCE_DIR}/src/services/accesscontrol/accesscontroldbus.h AccessControlDBus)
target_sources(ut-dfmdaemon-accesscontrol PRIVATE ${UT_ACC_ADAPTOR_SRC})

# 服务源码 unity-include 进测试 TU 后，把服务的 Q_OBJECT 头加入目标源列表，
# 由 AUTOMOC 为其生成元对象代码。
set(UT_ACC_MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/services/accesscontrol/accesscontroldbus.h
)
target_sources(ut-dfmdaemon-accesscontrol PRIVATE ${UT_ACC_MOC_HEADERS})

target_link_libraries(ut-dfmdaemon-accesscontrol PRIVATE
    Qt6::DBus
    Qt6::Concurrent
    Dtk6::Core
    PkgConfig::UT_ACC_CRYPT
    PkgConfig::UT_ACC_POLKIT
)
