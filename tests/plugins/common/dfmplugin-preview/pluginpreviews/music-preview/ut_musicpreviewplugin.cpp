// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "musicpreviewplugin.h"

#include <gtest/gtest.h>

PREVIEW_USE_NAMESPACE

TEST(UT_musicPreviewPlugin, create)
{
    MusicPreviewPlugin plugin;
    EXPECT_NO_FATAL_FAILURE(plugin.create("UT_TEST"));
}
