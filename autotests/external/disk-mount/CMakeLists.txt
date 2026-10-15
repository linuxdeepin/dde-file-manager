# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Unit tests for src/external/dde-dock-plugins/disk-mount utility sources.
# Recompiles dockutils.cpp into the test binary (pure functions, no dock deps).

set(_util_dir "${DFM_SOURCE_DIR}/external/dde-dock-plugins/disk-mount/utils")

file(GLOB _ut_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

dfm_add_test(ut-dockutils
    SOURCES ${_ut_files} ${_util_dir}/dockutils.cpp
    LINK_LIBRARIES DFM6::base Qt6::Widgets
)

target_include_directories(ut-dockutils PRIVATE
    ${_util_dir}
    ${DFM_SOURCE_DIR}/dfm-base/dbusservice
)

# ---------------------------------------------------------------------------
# Appended (batch external): pull the disk-mount plugin sources themselves
# into ut-dockutils so widgets / device manager / plugin classes can be
# unit tested. DBus proxies are regenerated from the same XML the plugin
# uses; Q_OBJECT headers are listed for AUTOMOC; plugin qrc provides the
# fallback icons referenced by DiskPluginItem::updateIcon.
# ---------------------------------------------------------------------------
find_package(Qt6 COMPONENTS DBus REQUIRED)
find_package(Dtk6 COMPONENTS Widget Gui REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UTDiskMountDock REQUIRED dde-dock)

set(_plugin_dir "${DFM_SOURCE_DIR}/external/dde-dock-plugins/disk-mount")

qt6_add_dbus_interface(UT_DISKMOUNT_IFACE
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.Daemon.DeviceManager.xml"
    devicemanager_interface)
qt6_add_dbus_interface(UT_DISKMOUNT_IFACE
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.UsbRepair.xml"
    usbrepair_interface)

qt6_add_resources(UT_DISKMOUNT_QRC
    "${_plugin_dir}/resources.qrc"
    "${_plugin_dir}/resources/theme-icons.qrc")

target_sources(ut-dockutils PRIVATE
    ${_plugin_dir}/device/dockitemdatamanager.cpp
    ${_plugin_dir}/device/usbrepairproxy.cpp
    ${_plugin_dir}/diskmountplugin.cpp
    ${_plugin_dir}/widgets/deviceitem.cpp
    ${_plugin_dir}/widgets/devicelist.cpp
    ${_plugin_dir}/widgets/diskpluginitem.cpp
    ${_plugin_dir}/widgets/repairdialog.cpp
    ${_plugin_dir}/widgets/tipswidget.cpp
    ${UT_DISKMOUNT_IFACE}
    ${UT_DISKMOUNT_QRC}
    ${_plugin_dir}/device/dockitemdatamanager.h
    ${_plugin_dir}/device/usbrepairproxy.h
    ${_plugin_dir}/diskmountplugin.h
    ${_plugin_dir}/widgets/deviceitem.h
    ${_plugin_dir}/widgets/devicelist.h
    ${_plugin_dir}/widgets/diskpluginitem.h
    ${_plugin_dir}/widgets/repairdialog.h
    ${_plugin_dir}/widgets/tipswidget.h
)

set_target_properties(ut-dockutils PROPERTIES AUTOMOC ON)

target_include_directories(ut-dockutils PRIVATE
    ${_plugin_dir}
    ${_plugin_dir}/device
    ${_plugin_dir}/widgets
    ${CMAKE_CURRENT_BINARY_DIR}
    ${UTDiskMountDock_INCLUDE_DIRS}
)

target_link_libraries(ut-dockutils PRIVATE
    Qt6::DBus
    Qt6::Test
    Dtk6::Widget
    Dtk6::Gui
)
