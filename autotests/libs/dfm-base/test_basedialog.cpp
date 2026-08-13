// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basedialog.cpp
 * @brief Unit tests for BaseDialog (basedialog.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/dialogs/basedialog/basedialog.h>

#include <QString>
#include <QFont>
#include <QResizeEvent>
#include <QWidget>

using namespace dfmbase;

TEST(BaseDialogTest, ConstructDoesNotCrash)
{
    BaseDialog dlg;
    (void)dlg;
}

TEST(BaseDialogTest, DestructDoesNotCrash)
{
    {
        BaseDialog dlg;
    }
    SUCCEED();
}

TEST(BaseDialogTest, SetTitleDoesNotCrash)
{
    BaseDialog dlg;
    dlg.setTitle(QStringLiteral("Test Dialog"));
}

TEST(BaseDialogTest, SetTitleFontDoesNotCrash)
{
    BaseDialog dlg;
    QFont font;
    font.setBold(true);
    dlg.setTitleFont(font);
}

// ============================================================
// Additional coverage for BaseDialog
// ============================================================

TEST(BaseDialogTest, ResizeEventDoesNotCrash)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(400, 300), QSize(300, 200));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}

TEST(BaseDialogTest, ResizeEventWithLargerSize)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}

TEST(BaseDialogTest, SetTitleMultipleTimes)
{
    BaseDialog dlg;
    dlg.setTitle(QStringLiteral("First Title"));
    dlg.setTitle(QStringLiteral("Second Title"));
    dlg.setTitle(QStringLiteral("Third Title"));
    SUCCEED();
}

TEST(BaseDialogTest, SetTitleEmpty)
{
    BaseDialog dlg;
    dlg.setTitle(QString());
    SUCCEED();
}

TEST(BaseDialogTest, SetTitleWithSpecialChars)
{
    BaseDialog dlg;
    dlg.setTitle(QStringLiteral("Test <b>Bold</b> & \"Quotes\""));
    SUCCEED();
}

TEST(BaseDialogTest, SetTitleFontWithDifferentSizes)
{
    BaseDialog dlg;
    QFont font;
    font.setPointSize(14);
    dlg.setTitleFont(font);
    font.setPointSize(10);
    dlg.setTitleFont(font);
    font.setItalic(true);
    dlg.setTitleFont(font);
    SUCCEED();
}

TEST(BaseDialogTest, ResizeEventSameSize)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(300, 200), QSize(300, 200));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}

TEST(BaseDialogTest, ResizeEventZeroSize)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(0, 0), QSize(0, 0));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}

TEST(BaseDialogTest, ConstructWithParent)
{
    QWidget parent;
    BaseDialog dlg(&parent);
    EXPECT_EQ(dlg.parent(), &parent);
}

TEST(BaseDialogTest, SetTitleChinese)
{
    BaseDialog dlg;
    dlg.setTitle(QStringLiteral("测试标题"));
    SUCCEED();
}

TEST(BaseDialogTest, SetTitleFontWithFamily)
{
    BaseDialog dlg;
    QFont font("Sans Serif", 12);
    dlg.setTitleFont(font);
    SUCCEED();
}

TEST(BaseDialogTest, ResizeEventWithZeroOldSize)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(500, 400), QSize(0, 0));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}

TEST(BaseDialogTest, SetTitleVeryLong)
{
    BaseDialog dlg;
    dlg.setTitle(QString(1000, 'A'));
    SUCCEED();
}

TEST(BaseDialogTest, ResizeEventWithVerySmallSize)
{
    BaseDialog dlg;
    QResizeEvent event(QSize(1, 1), QSize(10, 10));
    EXPECT_NO_FATAL_FAILURE({ dlg.resizeEvent(&event); });
}
