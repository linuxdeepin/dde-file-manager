// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionentry.cpp
 * @brief Unit tests for DCustomActionEntry Mid-priority methods (dfmplugin-menu)
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

class DCustomActionEntryTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(DCustomActionEntryTest, data)
{
    // Instance method data
    DCustomActionEntry obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.data(); (void)r; });
}

TEST_F(DCustomActionEntryTest, excludeMimeTypes)
{
    // Instance method excludeMimeTypes
    DCustomActionEntry obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.excludeMimeTypes(); (void)r; });
}

TEST_F(DCustomActionEntryTest, mimeTypes)
{
    // Instance method mimeTypes
    DCustomActionEntry obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.mimeTypes(); (void)r; });
}

TEST_F(DCustomActionEntryTest, notShowIn)
{
    // Instance method notShowIn
    DCustomActionEntry obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.notShowIn(); (void)r; });
}

TEST_F(DCustomActionEntryTest, DCustomActionEntry)
{
    // DCustomActionEntry
    SUCCEED();
}
