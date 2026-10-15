cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-sharecontrol" "${DFM_SOURCE_DIR}/src/services/sharecontrol")

# ---------------------------------------------------------------------------
# sharecontrol service sources are unity-included by the test TU (the service
# has no library target to link against), so provide the same dependency set
# the real service uses (see src/services/sharecontrol/dependencies.cmake).
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus)
pkg_check_modules(UT_SHARE_POLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)

target_include_directories(ut-dfmdaemon-sharecontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/sharecontrol
    ${CMAKE_SOURCE_DIR}/src/services/common
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate the DBus adaptor the service source includes
# ("sharecontroladaptor.h", class UserShareManagerAdaptor).
qt6_add_dbus_adaptor(UT_SHARE_ADAPTOR_SRC
    "${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.ShareControl.xml"
    ${CMAKE_SOURCE_DIR}/src/services/sharecontrol/sharecontroldbus.h ShareControlDBus)
target_sources(ut-dfmdaemon-sharecontrol PRIVATE ${UT_SHARE_ADAPTOR_SRC})

# 服务源码 unity-include 进测试 TU 后，把服务的 Q_OBJECT 头加入目标源列表，
# 由 AUTOMOC 为其生成元对象代码。
set(UT_SHARE_MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/services/sharecontrol/sharecontroldbus.h
)
target_sources(ut-dfmdaemon-sharecontrol PRIVATE ${UT_SHARE_MOC_HEADERS})

target_link_libraries(ut-dfmdaemon-sharecontrol PRIVATE
    Qt6::DBus
    PkgConfig::UT_SHARE_POLKIT
)
