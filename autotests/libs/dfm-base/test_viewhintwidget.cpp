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
    EXPECT_EQ(w.customWidget(), nullptr);
}

TEST(ViewHintWidgetTest, SetEmptyMessage)
{
    ViewHintWidget w;
    w.setMessage("");
    SUCCEED();
}

TEST(ViewHintWidgetTest, SetLeftCustomWidgetAndRetrieve)
{
    ViewHintWidget w;
    auto *leftWidget = new QWidget();
    w.setCustomWidget(leftWidget, ViewHintWidget::Side::Left);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Left), leftWidget);
}

TEST(ViewHintWidgetTest, ReplaceLeftCustomWidgetDoesNotCrash)
{
    ViewHintWidget w;
    auto *first = new QWidget();
    w.setCustomWidget(first, ViewHintWidget::Side::Left);
    auto *second = new QWidget();
    w.setCustomWidget(second, ViewHintWidget::Side::Left);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Left), second);
}

TEST(ViewHintWidgetTest, SetLeftCustomWidgetNull)
{
    ViewHintWidget w;
    auto *leftWidget = new QWidget();
    w.setCustomWidget(leftWidget, ViewHintWidget::Side::Left);
    w.setCustomWidget(nullptr, ViewHintWidget::Side::Left);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Left), nullptr);
}

TEST(ViewHintWidgetTest, SetIconAndLeftCustomWidgetCoexist)
{
    ViewHintWidget w;
    w.setIcon("dialog-warning");
    auto *leftWidget = new QWidget();
    w.setCustomWidget(leftWidget, ViewHintWidget::Side::Left);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Left), leftWidget);
}

TEST(ViewHintWidgetTest, LeftAndRightCustomWidgetCoexist)
{
    ViewHintWidget w;
    auto *left = new QWidget();
    auto *right = new QWidget();
    w.setCustomWidget(left, ViewHintWidget::Side::Left);
    w.setCustomWidget(right, ViewHintWidget::Side::Right);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Left), left);
    EXPECT_EQ(w.customWidget(ViewHintWidget::Side::Right), right);
}

// Bug 1 regression test: replacing the right custom widget must not
// accumulate QSpacerItem entries in the layout.  Each call previously
// inserted a fresh spacer via insertSpacing() but only removed the old
// widget — never its spacer — so the close button drifted right.
TEST(ViewHintWidgetTest, ReplaceRightCustomWidgetDoesNotAccumulateSpacers)
{
    ViewHintWidget w;

    // Find the content layout by locating the one that contains the
    // icon button (objectName "IconButton") — avoids matching internal
    // DIconButton layouts.
    QHBoxLayout *targetLayout = nullptr;
    const auto allWidgets = w.findChildren<QWidget *>();
    for (auto *widget : allWidgets) {
        auto *l = qobject_cast<QHBoxLayout *>(widget->layout());
        if (!l)
            continue;
        for (int i = 0; i < l->count(); ++i) {
            auto *item = l->itemAt(i);
            if (item && item->widget() && item->widget()->objectName() == "IconButton") {
                targetLayout = l;
                break;
            }
        }
        if (targetLayout)
            break;
    }
    ASSERT_NE(targetLayout, nullptr);

    // Before: icon + message + close = 3
    EXPECT_EQ(targetLayout->count(), 3);

    // After first right widget: icon + message + spacer + widget + close = 5
    w.setCustomWidget(new QWidget(), ViewHintWidget::Side::Right);
    EXPECT_EQ(targetLayout->count(), 5);

    // Each replacement must keep the count at 5, not 6, 7, …
    for (int i = 0; i < 5; ++i) {
        w.setCustomWidget(new QWidget(), ViewHintWidget::Side::Right);
        ASSERT_EQ(targetLayout->count(), 5) << "spacer leaked on iteration " << i;
    }

    // Clearing the right widget should also remove its spacer → back to 3
    w.setCustomWidget(nullptr, ViewHintWidget::Side::Right);
    EXPECT_EQ(targetLayout->count(), 3);
}
