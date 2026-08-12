// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_aliascombobox.cpp
 * @brief Unit tests for AliasComboBox (aliascombobox.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/dialogs/settingsdialog/controls/aliascombobox.h>

#include <QString>

TEST(AliasComboBoxTest, ConstructDoesNotCrash)
{
    AliasComboBox cb;
    (void)cb;
}

TEST(AliasComboBoxTest, SetAndGetItemAlias)
{
    AliasComboBox cb;
    cb.addItem(QStringLiteral("Item 1"));
    cb.addItem(QStringLiteral("Item 2"));
    cb.setItemAlias(0, QStringLiteral("Alias 1"));
    EXPECT_EQ(cb.itemAlias(0), QStringLiteral("Alias 1"));
}

TEST(AliasComboBoxTest, ItemAliasEmptyWhenNotSet)
{
    AliasComboBox cb;
    cb.addItem(QStringLiteral("Item 1"));
    EXPECT_TRUE(cb.itemAlias(0).isEmpty());
}

TEST(AliasComboBoxTest, OverwriteItemAlias)
{
    AliasComboBox cb;
    cb.addItem(QStringLiteral("Item 1"));
    cb.setItemAlias(0, QStringLiteral("first"));
    cb.setItemAlias(0, QStringLiteral("second"));
    EXPECT_EQ(cb.itemAlias(0), QStringLiteral("second"));
}
