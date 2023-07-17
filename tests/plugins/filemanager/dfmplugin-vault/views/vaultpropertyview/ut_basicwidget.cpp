// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vaultpropertyview/basicwidget.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_BasicWidget, selectFileUrl)
{
    BasicWidget widget;
    widget.selectFileUrl(QUrl("file:///UT_test"));

    EXPECT_FALSE(widget.fileCount->isVisible());
}

TEST(UT_BasicWidget, getFileSize)
{
    BasicWidget widget;
    widget.slotFileCountAndSizeChange(10, 11, 12);

    EXPECT_TRUE(widget.getFileSize() == 10);
}

TEST(UT_BasicWidget, getFileCount)
{
    BasicWidget widget;
    widget.slotFileCountAndSizeChange(10, 11, 12);

    EXPECT_TRUE(widget.getFileCount() == 22);
}
