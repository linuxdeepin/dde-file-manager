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
