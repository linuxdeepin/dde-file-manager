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
#include <QApplication>
#include <QPaintEvent>

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

// ============================================================
// Additional coverage for AliasComboBox
// ============================================================

TEST(AliasComboBoxTest, PaintEventDoesNotCrash)
{
    AliasComboBox cb;
    cb.addItem("Item A");
    cb.setItemAlias(0, "Alias A");
    cb.setCurrentIndex(0);
    cb.setFixedSize(200, 30);
    QPaintEvent evt(QRect(0, 0, 200, 30));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&cb, &evt); });
}

TEST(AliasComboBoxTest, PaintEventNoAlias)
{
    AliasComboBox cb;
    cb.addItem("Plain Item");
    cb.setCurrentIndex(0);
    cb.setFixedSize(200, 30);
    QPaintEvent evt(QRect(0, 0, 200, 30));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&cb, &evt); });
}

TEST(AliasComboBoxTest, PaintEventEmpty)
{
    AliasComboBox cb;
    cb.setFixedSize(200, 30);
    QPaintEvent evt(QRect(0, 0, 200, 30));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&cb, &evt); });
}

TEST(AliasComboBoxTest, ItemAliasOutOfRange)
{
    AliasComboBox cb;
    cb.addItem("A");
    // Out of range
    EXPECT_TRUE(cb.itemAlias(-1).isEmpty());
    EXPECT_TRUE(cb.itemAlias(999).isEmpty());
}

TEST(AliasComboBoxTest, SetItemAliasOutOfRange)
{
    AliasComboBox cb;
    cb.addItem("A");
    EXPECT_NO_FATAL_FAILURE({ cb.setItemAlias(-1, "bad"); });
}
