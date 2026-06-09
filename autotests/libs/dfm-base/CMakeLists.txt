# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# dfm-base unit tests
# Stub DeviceProxyManager to avoid DBus/system dependencies.

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

dfm_add_test(test-dfm-base
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES DFM6::base Qt6::Test
)
