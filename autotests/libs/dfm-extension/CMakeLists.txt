# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# dfm-extension unit tests — one suite per source submodule.
# Mirrors src/dfm-extension/{emblemicon,file,menu,window} plus the global
# extension header. Incremental CI (run-ut.sh --changed-files) selects only
# the affected suite. All suites link the prebuilt DFM::extension library;
# dfm_extension_stub.cpp provides stub implementations for the plugin C API.

set(EXT_SUBMODULES
    emblemicon
    file
    menu
    window
    global
)

foreach(_mod ${EXT_SUBMODULES})
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

    dfm_add_test(test-dfm-extension-${_mod}
        SOURCES ${_mod_sources}
                "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/dfm_extension_stub.cpp"
        LINK_LIBRARIES DFM::extension Qt6::Test
        COVERS src/dfm-extension/${_mod}
    )
endforeach()
