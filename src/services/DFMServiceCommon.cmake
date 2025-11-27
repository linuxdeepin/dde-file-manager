# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Setup service polkit helper for existing target (after add_library)
# Usage for target mode (in dependencies.cmake):
#   function(dfm_setup_xxx_dependencies target_name)
#       dfm_apply_service_polkit_to_target(${target_name})
#       ...
#   endfunction()
function(dfm_apply_service_polkit_to_target target_name)
    message(STATUS "DFM: Applying shared polkit helper to target: ${target_name}")

    # Add common polkit helper implementation
    target_sources(${target_name} PRIVATE
        ${DFM_SOURCE_DIR}/services/common/polkit/policykithelper.cpp
    )

    # Include common directory for shared headers
    target_include_directories(${target_name} PRIVATE
        ${DFM_SOURCE_DIR}/services/common
    )

    message(STATUS "DFM: Shared polkit helper applied successfully")
endfunction()
