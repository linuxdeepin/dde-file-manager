// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/filepreviewdialogstatusbar.h"

#include <gtest/gtest.h>

DPFILEPREVIEW_USE_NAMESPACE

TEST(UT_FilePreviewDialogStatusBar, title)
{
    FilePreviewDialogStatusBar bar;

    EXPECT_TRUE(bar.title());
}

TEST(UT_FilePreviewDialogStatusBar, preButton)
{
    FilePreviewDialogStatusBar bar;

    EXPECT_TRUE(bar.preButton());
}

TEST(UT_FilePreviewDialogStatusBar, nextButton)
{
    FilePreviewDialogStatusBar bar;

    EXPECT_TRUE(bar.nextButton());
}

TEST(UT_FilePreviewDialogStatusBar, openButton)
{
    FilePreviewDialogStatusBar bar;

    EXPECT_TRUE(bar.openButton());
}
