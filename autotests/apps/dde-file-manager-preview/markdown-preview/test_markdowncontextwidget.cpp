// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "markdowncontextwidget.h"
#include "markdownbrowser.h"

#include <gtest/gtest.h>

#include <QMargins>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

using namespace plugin_filepreview;

class UT_MarkdownContextWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        widget = new MarkdownContextWidget();
    }

    virtual void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

protected:
    MarkdownContextWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_MarkdownContextWidget, Constructor_DefaultArguments_CreatesBrowserChild)
{
    auto browser = widget->markdownBrowser();
    EXPECT_NE(nullptr, browser);
    EXPECT_EQ(widget, browser->parentWidget());
    EXPECT_NE(nullptr, widget->findChild<MarkdownBrowser *>());
}

TEST_F(UT_MarkdownContextWidget, Constructor_DefaultArguments_BuildsZeroMarginVBoxLayout)
{
    auto layout = qobject_cast<QVBoxLayout *>(widget->layout());
    EXPECT_NE(nullptr, layout);
    EXPECT_EQ(layout->contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(layout->spacing(), 0);
    EXPECT_EQ(layout->count(), 2);
}

TEST_F(UT_MarkdownContextWidget, Constructor_DefaultArguments_PlacesReadOnlyTitleAboveBrowser)
{
    auto layout = qobject_cast<QVBoxLayout *>(widget->layout());
    ASSERT_NE(nullptr, layout);

    auto plainEdits = widget->findChildren<QPlainTextEdit *>();
    ASSERT_EQ(plainEdits.size(), 1);

    EXPECT_EQ(layout->itemAt(0)->widget(), plainEdits.first());
    EXPECT_EQ(layout->itemAt(1)->widget(), widget->markdownBrowser());
    EXPECT_TRUE(plainEdits.first()->isReadOnly());
    EXPECT_EQ(plainEdits.first()->minimumHeight(), 30);
    EXPECT_EQ(plainEdits.first()->maximumHeight(), 30);
}

TEST_F(UT_MarkdownContextWidget, Constructor_WithParentWidget_ParentAssigned)
{
    QWidget parent;
    auto child = new MarkdownContextWidget(&parent);
    EXPECT_EQ(child->parentWidget(), &parent);
    EXPECT_NE(nullptr, child->markdownBrowser());
    delete child;
}

TEST_F(UT_MarkdownContextWidget, MarkdownBrowser_RepeatedCalls_ReturnSameInstance)
{
    auto first = widget->markdownBrowser();
    auto second = widget->markdownBrowser();
    EXPECT_NE(nullptr, first);
    EXPECT_EQ(first, second);
    EXPECT_EQ(widget->findChild<MarkdownBrowser *>(), first);
}
