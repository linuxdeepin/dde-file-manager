# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Setup service common resources (e.g. shared PolicyKitHelper)
# Usage:
#   FILE(GLOB YOUR_FILES ...)
#   dfm_setup_service_polkit(YOUR_FILES)
#   add_library(${PROJECT_NAME} SHARED ${YOUR_FILES})
function(dfm_setup_service_polkit FILES_VAR)
    # Exclude old polkit files from individual services
    list(FILTER ${FILES_VAR} EXCLUDE REGEX "polkit/policykithelper\\.(h|cpp)$")

    # Add common polkit helper implementation
    list(APPEND ${FILES_VAR} "${CMAKE_CURRENT_SOURCE_DIR}/../common/polkit/policykithelper.cpp")

    # Return modified list to caller
    set(${FILES_VAR} ${${FILES_VAR}} PARENT_SCOPE)

    # Include common directory for shared headers
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)
endfunction()
