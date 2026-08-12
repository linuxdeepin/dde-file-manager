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
