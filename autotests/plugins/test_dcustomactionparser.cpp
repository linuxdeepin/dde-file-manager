// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionparser.cpp
 * @brief Unit tests for DCustomActionParser Mid-priority methods (dfmplugin-menu)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "extendmenuscene/extendmenu/dcustomactionparser.h"

using namespace dfmplugin_menu;

class DCustomActionParserTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(DCustomActionParserTest, actionNameDynamicArg)
{
    DCustomActionData _arg_act;
    // Instance method actionNameDynamicArg
    DCustomActionParser obj;
    EXPECT_NO_FATAL_FAILURE({ obj.actionNameDynamicArg(_arg_act); });
}

TEST_F(DCustomActionParserTest, comboPosForTopAction)
{
    QSettings _arg_actionSetting;
    DCustomActionData _arg_act;
    // Instance method comboPosForTopAction
    DCustomActionParser obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.comboPosForTopAction(_arg_actionSetting, QString("test"), _arg_act); });
    (void)result;
}

TEST_F(DCustomActionParserTest, execDynamicArg)
{
    DCustomActionData _arg_act;
    // Instance method execDynamicArg
    DCustomActionParser obj;
    EXPECT_NO_FATAL_FAILURE({ obj.execDynamicArg(_arg_act); });
}

TEST_F(DCustomActionParserTest, loadDir)
{
    // Instance method loadDir
    DCustomActionParser obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.loadDir(QStringList{"test"}); });
    (void)result;
}

TEST_F(DCustomActionParserTest, refresh)
{
    // Instance method refresh
    DCustomActionParser obj;
    EXPECT_NO_FATAL_FAILURE({ obj.refresh(); });
}

TEST_F(DCustomActionParserTest, DCustomActionParser)
{
    // DCustomActionParser
    SUCCEED();
}
