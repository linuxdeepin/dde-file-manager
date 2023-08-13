// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "textcontextwidget.h"

#include <gtest/gtest.h>

using namespace plugin_filepreview;

TEST(UT_textContextWidget, textBrowserEdit)
{
    TextContextWidget widget;
    EXPECT_TRUE(widget.textBrowserEdit());
}
