// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_ASAN_HELPER_H
#define DFM_ASAN_HELPER_H

#ifdef ENABLE_TSAN_TOOL
#include <sanitizer/asan_interface.h>

/**
 * @brief Setup ASAN report path with standardized naming
 * 
 * This macro sets up the ASAN report path in a standardized format:
 * - File name: "asan_<project_name>.log"
 * - Location: Absolute path defined by CMake (DFM_ASAN_REPORT_DIR)
 * 
 * @param project_name The name of the project (without quotes)
 * 
 * Usage in main.cpp:
 * ```cpp
 * #include "dfm_asan_helper.h"
 * 
 * int main(int argc, char *argv[]) {
 *     DFM_SETUP_ASAN_REPORT(dfm_extension);
 *     
 *     // ... other initialization code ...
 *     
 *     int ret = RUN_ALL_TESTS();
 *     return ret;
 * }
 * ```
 */
#define DFM_SETUP_ASAN_REPORT(project_name) \
    do { \
        /* Construct absolute path for ASAN report */ \
        std::string asan_report_path = std::string(DFM_ASAN_REPORT_DIR) + "/asan_" #project_name ".log"; \
        /* Set ASAN report path */ \
        __sanitizer_set_report_path(asan_report_path.c_str()); \
        std::cout << "DFM ASAN Helper: Report path set to " << asan_report_path << std::endl; \
    } while(0)

#else
// No-op when ASAN is not enabled
#define DFM_SETUP_ASAN_REPORT(project_name) do { } while(0)
#endif

#endif // DFM_ASAN_HELPER_H 
