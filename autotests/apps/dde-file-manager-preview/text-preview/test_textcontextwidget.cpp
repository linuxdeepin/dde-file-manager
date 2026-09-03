// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textcontextwidget.h"
#include "textbrowseredit.h"

#include <dfm-base/dfm_log_defines.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QLayout>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>

using namespace plugin_filepreview;

class UT_TextContextWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        widget = new TextContextWidget();
    }

    virtual void TearDown() override
    {
        delete widget;
        widget = nullptr;
    }

protected:
    TextContextWidget *widget { nullptr };
};

TEST_F(UT_TextContextWidget, Constructor_DefaultArgs_TitleAndEditWidgetsLaidOut)
{
    auto *layout = qobject_cast<QVBoxLayout *>(widget->layout());
    ASSERT_TRUE(layout);
    ASSERT_EQ(layout->count(), 2);
    EXPECT_EQ(layout->contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(layout->spacing(), 0);

    auto *titleWidget = qobject_cast<QPlainTextEdit *>(layout->itemAt(0)->widget());
    ASSERT_TRUE(titleWidget);
    EXPECT_EQ(titleWidget->minimumHeight(), 30);
    EXPECT_EQ(titleWidget->maximumHeight(), 30);
    EXPECT_TRUE(titleWidget->isReadOnly());
    EXPECT_EQ(titleWidget->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_EQ(titleWidget->frameShape(), QFrame::NoFrame);

    auto *editWidget = qobject_cast<TextBrowserEdit *>(layout->itemAt(1)->widget());
    ASSERT_TRUE(editWidget);
    EXPECT_EQ(editWidget, widget->textBrowserEdit());
    EXPECT_NE(titleWidget, editWidget);
}

TEST_F(UT_TextContextWidget, TextBrowserEdit_AfterConstruction_ReturnsNonNullInstance)
{
    EXPECT_NE(widget->textBrowserEdit(), nullptr);
}

TEST_F(UT_TextContextWidget, TextBrowserEdit_RepeatedCalls_ReturnsSameInstance)
{
    TextBrowserEdit *first = widget->textBrowserEdit();
    TextBrowserEdit *second = widget->textBrowserEdit();
    EXPECT_EQ(first, second);
}

TEST_F(UT_TextContextWidget, TextBrowserEdit_InstanceIsChildOfContextWidget)
{
    EXPECT_EQ(widget->textBrowserEdit()->parentWidget(), widget);
}
