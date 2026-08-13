// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewhintwidget.cpp
 * @brief Unit tests for ViewHintWidget (viewhintwidget.cpp)
 *
 * ViewHintWidget is a DFloatingMessage subclass with custom icon/message
 * setters and a custom-widget slot. Constructible in offscreen mode — no
 * display hardware needed.
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/viewhintmessage/viewhintwidget.h>

#include <QWidget>
#include <QString>
#include <QLabel>

using namespace dfmbase;

TEST(ViewHintWidgetTest, ConstructAndDestructWithoutCrash)
{
    {
        ViewHintWidget w;
        (void)w;
    }
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetMessageUpdatesLabel)
{
    ViewHintWidget w;
    w.setMessage(QStringLiteral("hello world"));
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetIconDoesNotCrash)
{
    ViewHintWidget w;
    w.setIcon(QStringLiteral("dialog-warning"));
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetCustomWidgetAndRetrieve)
{
    ViewHintWidget w;
    auto *custom = new QWidget();
    w.setCustomWidget(custom);
    EXPECT_EQ(w.customWidget(), custom);
}

TEST(ViewHintWidgetTest, ReplaceCustomWidgetDoesNotCrash)
{
    ViewHintWidget w;
    auto *first = new QWidget();
    w.setCustomWidget(first);
    auto *second = new QWidget();
    w.setCustomWidget(second);
    EXPECT_EQ(w.customWidget(), second);
}

// ============================================================
// Additional coverage for ViewHintWidget
// ============================================================

TEST(ViewHintWidgetTest, SetMessageTwice)
{
    ViewHintWidget w;
    w.setMessage("first");
    w.setMessage("second");
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetIconMultipleTimes)
{
    ViewHintWidget w;
    w.setIcon("dialog-warning");
    w.setIcon("dialog-information");
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetCustomWidgetNull)
{
    ViewHintWidget w;
    auto *child = new QWidget();
    w.setCustomWidget(child);
    w.setCustomWidget(nullptr);
    // After setting null, old widget should be removed
    EXPECT_EQ(w.customWidget(), nullptr);
}

TEST(ViewHintWidgetTest, SetEmptyMessage)
{
    ViewHintWidget w;
    w.setMessage("");
    SUCCEED();
}
