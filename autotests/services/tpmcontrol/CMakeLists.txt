cmake_minimum_required(VERSION 3.10)

# Use DFM enhanced test utilities to create service test
dfm_create_plugin_test_enhanced("dfmdaemon-tpmcontrol" "${DFM_SOURCE_DIR}/src/services/tpmcontrol")

# ---------------------------------------------------------------------------
# PMS sev-2 regression tests: tpmcontrol service sources are unity-included
# by the test TU (service is built as an executable, not a library), so the
# same dependency set as the real service is required here.
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus)
pkg_check_modules(UT_TPMPOLKIT REQUIRED IMPORTED_TARGET polkit-qt6-1)

# Q_OBJECT 头需加入目标源列表以便 AUTOMOC 生成元对象代码
target_sources(ut-dfmdaemon-tpmcontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/tpmcontrol/tpmcontroldbus.h
    ${CMAKE_SOURCE_DIR}/src/services/tpmcontrol/core/tpmwork.h
)

target_include_directories(ut-dfmdaemon-tpmcontrol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services/tpmcontrol
    ${CMAKE_SOURCE_DIR}/src/services/common
)

target_link_libraries(ut-dfmdaemon-tpmcontrol PRIVATE
    Qt6::DBus
    PkgConfig::UT_TPMPOLKIT
)
