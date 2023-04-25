// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appendcompress/virtualappendcompressplugin.h"

#include <gtest/gtest.h>

DPUTILS_USE_NAMESPACE

TEST(UT_UtilsPluginBugTest, bug_123879_CheckCompressionFunc)
{
    VirtualAppendCompressPlugin plugin;
    EXPECT_TRUE(plugin.start());
}
