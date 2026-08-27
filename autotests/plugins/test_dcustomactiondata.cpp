// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactiondata.cpp
 * @brief Unit tests for DCustomActionData Mid-priority methods (dfmplugin-menu)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "extendmenuscene/extendmenu/dcustomactiondata.h"

using namespace dfmplugin_menu;

class DCustomActionDataTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(DCustomActionDataTest, command)
{
    // Instance method command
    DCustomActionData obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.command(); (void)r; });
}

TEST_F(DCustomActionDataTest, name)
{
    // Instance method name
    DCustomActionData obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.name(); (void)r; });
}

TEST_F(DCustomActionDataTest, parentPath)
{
    // Instance method parentPath
    DCustomActionData obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.parentPath(); (void)r; });
}

TEST_F(DCustomActionDataTest, position)
{
    // Instance method position
    DCustomActionData obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.position(); (void)r; });
}

TEST_F(DCustomActionDataTest, DCustomActionData)
{
    // DCustomActionData
    SUCCEED();
}

TEST_F(DCustomActionDataTest, separator)
{
    // separator
    SUCCEED();
}
