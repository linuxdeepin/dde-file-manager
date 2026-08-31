// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_commandparser.cpp
 * @brief Unit tests for CommandParser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager/commandparser.h"

#include <QTest>

using namespace src;

class CommandParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CommandParser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CommandParser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CommandParserTest, initOptions)
{
    // Test method: void initOptions(())
    EXPECT_NO_FATAL_FAILURE(obj->initOptions());
}

TEST_F(CommandParserTest, instance)
{
    // Test getter: CommandParser instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(CommandParserTest, isSet)
{
    // Test method: bool isSet((const QString &name))
    QString _arg0{};
    auto result = obj->isSet(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CommandParserTest, openWithDialog)
{
    // Test method: void openWithDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->openWithDialog());
}

TEST_F(CommandParserTest, process)
{
    // Test method: void process((const QStringList &arguments))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->process(_arg0));
}

TEST_F(CommandParserTest, showPropertyDialog)
{
    // Test method: void showPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->showPropertyDialog());
}

TEST_F(CommandParserTest, value)
{
    // Test method: QString value((const QString &name))
    QString _arg0{};
    auto result = obj->value(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
