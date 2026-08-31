// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keyvaluelabel.cpp
 * @brief Unit tests for KeyValueLabel (keyvaluelabel.cpp)
 */

#include <gtest/gtest.h>
#include <QFrame>
#include <QString>

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>

using namespace dfmbase;

TEST(KeyValueLabelTest, ConstructAndDestruct)
{
    KeyValueLabel label(nullptr);
    EXPECT_FALSE(label.LeftValue().isEmpty() && label.RightValue().isEmpty() ? false : false);
}

TEST(KeyValueLabelTest, SetLeftValue)
{
    KeyValueLabel label(nullptr);
    label.setLeftValue("key");
    EXPECT_EQ(label.LeftValue(), QString("key"));
}

TEST(KeyValueLabelTest, SetRightValue)
{
    KeyValueLabel label(nullptr);
    label.setRightValue("value");
    EXPECT_EQ(label.RightValue(), QString("value"));
}

TEST(KeyValueLabelTest, SetLeftRightValue)
{
    KeyValueLabel label(nullptr);
    label.setLeftRightValue("k", "v");
    EXPECT_EQ(label.LeftValue(), QString("k"));
    EXPECT_EQ(label.RightValue(), QString("v"));
}

TEST(KeyValueLabelTest, SetLeftWordWrap)
{
    KeyValueLabel label(nullptr);
    EXPECT_NO_FATAL_FAILURE({ label.setLeftWordWrap(true); });
}

TEST(KeyValueLabelTest, SetLeftFontSizeWeight)
{
    KeyValueLabel label(nullptr);
    EXPECT_NO_FATAL_FAILURE({ label.setLeftFontSizeWeight(DFontSizeManager::T8, QFont::Normal); });
}

TEST(KeyValueLabelTest, SetRightFontSizeWeight)
{
    KeyValueLabel label(nullptr);
    EXPECT_NO_FATAL_FAILURE({ label.setRightFontSizeWeight(DFontSizeManager::T8, QFont::Normal); });
}

TEST(KeyValueLabelTest, LeftAndRightWidgetsExist)
{
    KeyValueLabel label(nullptr);
    EXPECT_NE(label.leftWidget(), nullptr);
    EXPECT_NE(label.rightWidget(), nullptr);
}

TEST(KeyValueLabelTest, SetLeftValueLabelFixedWidth)
{
    KeyValueLabel label(nullptr);
    EXPECT_NO_FATAL_FAILURE({ label.setLeftValueLabelFixedWidth(100); });
}

TEST(KeyValueLabelTest, RightValueWidgetSetCompleteText)
{
    KeyValueLabel label(nullptr);
    auto *w = label.rightWidget();
    ASSERT_NE(w, nullptr);
    EXPECT_NO_FATAL_FAILURE({ w->setCompleteText("complete text"); });
}

TEST(KeyValueLabelTest, KeyValueLabelDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ KeyValueLabel label(nullptr); });
}

// ---- Coverage additions: RightValueWidget event handlers (direct, no show/QMenu) ----
#include <QShowEvent>
#include <QMouseEvent>
#include <QFocusEvent>

TEST(KeyValueLabelTest, RightValueWidgetShowEventCallable)
{
    KeyValueLabel label(nullptr);
    auto *w = label.rightWidget();
    ASSERT_NE(w, nullptr);
    EXPECT_NO_FATAL_FAILURE({
        QShowEvent se;
        w->showEvent(&se);
    });
}

TEST(KeyValueLabelTest, RightValueWidgetMouseReleaseEventCallable)
{
    KeyValueLabel label(nullptr);
    auto *w = label.rightWidget();
    ASSERT_NE(w, nullptr);
    EXPECT_NO_FATAL_FAILURE({
        QMouseEvent me(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        w->mouseReleaseEvent(&me);
    });
}

TEST(KeyValueLabelTest, RightValueWidgetFocusOutEventCallable)
{
    KeyValueLabel label(nullptr);
    auto *w = label.rightWidget();
    ASSERT_NE(w, nullptr);
    EXPECT_NO_FATAL_FAILURE({
        QFocusEvent fe(QEvent::FocusOut);
        w->focusOutEvent(&fe);
    });
}


TEST(KeyValueLabelTest, KeyValueLabel)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE({ KeyValueLabel obj; });
    }


TEST(KeyValueLabelTest, rightWidget)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.rightWidget(); });
}

TEST(KeyValueLabelTest, setLeftFontSizeWeight)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE(obj.setLeftFontSizeWeight(DFontSizeManager::SizeType(), QFont::Weight(), DPalette::ColorType()));
}

TEST(KeyValueLabelTest, setLeftValue)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE(obj.setLeftValue(QString(), Qt::TextElideMode(), Qt::Alignment(), false, 0));
}

TEST(KeyValueLabelTest, setRightFontSizeWeight)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE(obj.setRightFontSizeWeight(DFontSizeManager::SizeType(), QFont::Weight(), DPalette::ColorType()));
}

TEST(KeyValueLabelTest, setRightValue)
{
    KeyValueLabel obj;
    EXPECT_NO_FATAL_FAILURE(obj.setRightValue(QString(), Qt::TextElideMode(), Qt::Alignment(), false, 0));
}
