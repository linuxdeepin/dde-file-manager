cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-usbrepair" "${DFM_SOURCE_DIR}/src/services/usbrepair")

# ---------------------------------------------------------------------------
# usbrepair service sources are unity-included by the test TU (the service has
# no library target to link against), so provide the same dependency set the
# real service uses (see src/services/usbrepair/dependencies.cmake).
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus)
pkg_check_modules(UT_USBREPAIR_POLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)

target_include_directories(ut-dfmdaemon-usbrepair PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate the DBus adaptor the service private header includes
# ("usbrepairadaptor.h", class UsbRepairAdaptor).
qt6_add_dbus_adaptor(UT_USBREPAIR_ADAPTOR_SRC
    "${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.UsbRepair.xml"
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair/usbrepairdbus.h UsbRepairDBus)
target_sources(ut-dfmdaemon-usbrepair PRIVATE ${UT_USBREPAIR_ADAPTOR_SRC})

# 服务源码 unity-include 进测试 TU 后，把服务的 Q_OBJECT 头加入目标源列表，
# 由 AUTOMOC 为其生成元对象代码。
set(UT_USBREPAIR_MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair/usbrepairdbus.h
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair/usbrepairmonitor.h
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair/usbrepairworker.h
    ${CMAKE_SOURCE_DIR}/src/services/usbrepair/private/usbrepairdbus_p.h
)
target_sources(ut-dfmdaemon-usbrepair PRIVATE ${UT_USBREPAIR_MOC_HEADERS})

target_link_libraries(ut-dfmdaemon-usbrepair PRIVATE
    Qt6::DBus
    PkgConfig::UT_USBREPAIR_POLKIT
)
