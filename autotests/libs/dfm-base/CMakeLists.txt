# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# dfm-base unit tests
# Stub DeviceProxyManager to avoid DBus/system dependencies.

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

dfm_add_test(ut-dfm-base
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES DFM6::base Qt6::Test
)

# PMS sev-2 regression tests (batch 2): stub QDBusConnection in test_fileutils.cpp
target_link_libraries(ut-dfm-base PRIVATE Qt6::DBus)
