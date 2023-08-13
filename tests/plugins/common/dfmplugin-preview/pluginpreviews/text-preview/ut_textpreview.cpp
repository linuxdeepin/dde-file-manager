// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "textpreview.h"
#include "textcontextwidget.h"

#include <gtest/gtest.h>

PREVIEW_USE_NAMESPACE

TEST(UT_textPreview, setFileUrl_one)
{
    TextPreview preview;
    QUrl url("file:///UT_TEST");
    preview.selectUrl = url;
    bool re = preview.setFileUrl(url);

    EXPECT_TRUE(re);
}

TEST(UT_textPreview, setFileUrl_two)
{
    TextPreview preview;
    QUrl url("file:///UT_TEST");
    bool re = preview.setFileUrl(url);

    EXPECT_FALSE(re);
}

TEST(UT_textPreview, fileUrl)
{
    QUrl url("file:///UT_TEST");
    TextPreview preview;
    preview.selectUrl = url;

    EXPECT_TRUE(url == preview.fileUrl());
}

TEST(UT_textPreview, contentWidget)
{
    TextContextWidget widget;
    TextPreview preview;
    preview.textBrowser = &widget;
    EXPECT_TRUE(preview.contentWidget() == &widget);

    preview.textBrowser = nullptr;
}

TEST(UT_textPreview, title)
{
    QString title("UT_TEST");
    TextPreview preview;
    preview.titleStr = title;

    EXPECT_TRUE(preview.title() == title);
}

TEST(UT_textPreview, showStatusBarSeparator)
{
    TextPreview preview;

    EXPECT_FALSE(preview.showStatusBarSeparator());
}
