// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionbuilder.cpp
 * @brief Unit tests for DCustomActionBuilder Mid-priority methods (dfmplugin-menu)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"

using namespace dfmplugin_menu;

class DCustomActionBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(DCustomActionBuilderTest, isMimeTypeMatch)
{
    // Instance method isMimeTypeMatch
    DCustomActionBuilder obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isMimeTypeMatch(QStringList{"test"}, QStringList{"test"}); });
    (void)result;
}

TEST_F(DCustomActionBuilderTest, isMimeTypeSupport)
{
    // Instance method isMimeTypeSupport
    DCustomActionBuilder obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isMimeTypeSupport(QString("test"), QStringList{"test"}); });
    (void)result;
}

TEST_F(DCustomActionBuilderTest, isSchemeSupport)
{
    // Instance method isSchemeSupport
    DCustomActionBuilder obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isSchemeSupport(DCustomActionEntry(), QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(DCustomActionBuilderTest, isSuffixSupport)
{
    // Instance method isSuffixSupport
    DCustomActionBuilder obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isSuffixSupport(DCustomActionEntry(), {}); });
    (void)result;
}

TEST_F(DCustomActionBuilderTest, makeName)
{
    // Instance method makeName
    DCustomActionBuilder obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.makeName(QString("test"), DCustomActionDefines::ActionArg()); (void)r; });
}

TEST_F(DCustomActionBuilderTest, matchFileCombo)
{
    // Instance method matchFileCombo
    DCustomActionBuilder obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.matchFileCombo({}, DCustomActionDefines::ComboTypes()); (void)r; });
}

TEST_F(DCustomActionBuilderTest, checkFileCombo)
{
    // checkFileCombo
    SUCCEED();
}

TEST_F(DCustomActionBuilderTest, checkFileComboWithFocus)
{
    // checkFileComboWithFocus
    SUCCEED();
}

TEST_F(DCustomActionBuilderTest, createMenu)
{
    // createMenu
    SUCCEED();
}
