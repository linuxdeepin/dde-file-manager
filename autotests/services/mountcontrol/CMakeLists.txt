cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-mountcontrol" "${DFM_SOURCE_DIR}/src/services/mountcontrol")

# ---------------------------------------------------------------------------
# PMS sev-2 regression tests: mountcontrol service sources are unity-included
# by the test TU (service is built as an executable, not a library), so the
# same dependency set as the real service is required here.
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus)
pkg_check_modules(UT_MNTPOLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)
pkg_check_modules(UT_MOUNT REQUIRED IMPORTED_TARGET mount)

target_include_directories(ut-dfmdaemon-mountcontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/mountcontrol
    ${CMAKE_SOURCE_DIR}/src/services/common
)

target_link_libraries(ut-dfmdaemon-mountcontrol PRIVATE
    Qt6::DBus
    PkgConfig::UT_MNTPOLKIT
    PkgConfig::UT_MOUNT
)

# 覆盖率补全 TU unity-include 了 mountcontroldbus.cpp，其 Q_OBJECT 头需加入
# 目标源列表以便 AUTOMOC 生成元对象代码（vtable/metacall）。
target_sources(ut-dfmdaemon-mountcontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/mountcontrol/mountcontroldbus.h
)

# 桩适配器头遵循 test_ 前缀命名；src/services/mountcontrol/private/mountcontroldbus_p.h
# 以生成器固定名 include "mountcontroladaptor.h"，构建期复制一份到构建目录供其解析。
configure_file(test_mountcontroladaptor.h
    "${CMAKE_CURRENT_BINARY_DIR}/mountcontroladaptor.h" COPYONLY)
target_include_directories(ut-dfmdaemon-mountcontrol PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
)
