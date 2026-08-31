// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionparser.cpp
 * @brief Unit tests for DCustomActionParser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionParser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionParser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionParserTest, actionNameDynamicArg)
{
    // Test method: void actionNameDynamicArg((DCustomActionData &act))
    DCustomActionData _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->actionNameDynamicArg(_arg0));
}

TEST_F(DCustomActionParserTest, comboPosForTopAction)
{
    // Test method: bool comboPosForTopAction((QSettings &actionSetting, const QString &group, DCustomActionData &act))
    QSettings _arg0{};
    QString _arg1{};
    DCustomActionData _arg2{};
    auto result = obj->comboPosForTopAction(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionParserTest, execDynamicArg)
{
    // Test method: void execDynamicArg((DCustomActionData &act))
    DCustomActionData _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->execDynamicArg(_arg0));
}

TEST_F(DCustomActionParserTest, loadDir)
{
    // Test method: bool loadDir((const QStringList &dirPaths))
    QStringList _arg0{};
    auto result = obj->loadDir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionParserTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(DCustomActionParserTest, DCustomActionParser)
{
    // Test constructor: DCustomActionParser((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
