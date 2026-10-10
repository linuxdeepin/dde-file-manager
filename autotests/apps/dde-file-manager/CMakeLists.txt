# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Unit tests for src/apps/dde-file-manager app-level sources.
# Recompiles the app sources under test into the test binary so stub-ext can
# intercept private/static dependencies (see autotests/README.md).

set(_app_dir "${DFM_SOURCE_DIR}/apps/dde-file-manager")

file(GLOB_RECURSE _ut_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(_app_sources
    "${_app_dir}/commandparser.cpp"
    "${_app_dir}/sessionloader.cpp"
)

find_package(Qt6 COMPONENTS Widgets Gui REQUIRED)

dfm_add_test(ut-commandparser
    SOURCES ${_ut_files} ${_app_sources}
    LINK_LIBRARIES DFM6::base DFM6::framework Qt6::Widgets Qt6::Gui
)

target_include_directories(ut-commandparser PRIVATE
    ${_app_dir}
)
