# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# dfm-framework unit tests — one suite per source submodule.
# Splitting by submodule lets incremental CI (run-ut.sh --changed-files)
# run only the suites affected by a change, e.g. editing
# src/dfm-framework/lifecycle only rebuilds/reruns test-dfm-framework-lifecycle.
# Each suite links the same prebuilt DFM6::framework shared library.

set(FW_SUBMODULES
    backtrace
    event
    lifecycle
    listener
    log
)

foreach(_mod ${FW_SUBMODULES})
    set(_mod_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_mod}")
    if(NOT EXISTS "${_mod_dir}")
        continue()
    endif()

    file(GLOB_RECURSE _mod_sources
        "${_mod_dir}/*.cpp"
        "${_mod_dir}/*.h"
    )
    if(NOT _mod_sources)
        continue()
    endif()

    dfm_add_test(test-dfm-framework-${_mod}
        SOURCES ${_mod_sources} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        LINK_LIBRARIES DFM6::framework Qt6::Test
        COVERS src/dfm-framework/${_mod}
    )
endforeach()
