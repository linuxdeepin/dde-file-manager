// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#ifdef ENABLE_TSAN_TOOL
#include <sanitizer/tsan_interface.h>
#endif

DFM_TEST_MAIN(dfmplugin_disk_encrypt_entry)
