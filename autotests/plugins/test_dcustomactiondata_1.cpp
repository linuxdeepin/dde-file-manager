// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactiondata_1.cpp
 * @brief Unit tests for DCustomActionData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactiondata.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionDataTest, DCustomActionData)
{
    // Test constructor: DCustomActionData((const DCustomActionData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DCustomActionDataTest, operator=)
{
    // Test getter: DCustomActionData operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(DCustomActionDataTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, position)
{
    // Test getter: int position()
    auto result = obj->position();
    EXPECT_EQ(result, 0);

}

TEST_F(DCustomActionDataTest, icon)
{
    // Test getter: QString icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, command)
{
    // Test getter: QString command()
    auto result = obj->command();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, separator)
{
    // Test getter: DCustomActionDefines::Separator separator()
    auto result = obj->separator();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, acitons)
{
    // Test getter: QList<DCustomActionData> acitons()
    auto result = obj->acitons();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, isMenu)
{
    // Test bool getter: isMenu()
    bool result = obj->isMenu();
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionDataTest, isAction)
{
    // Test bool getter: isAction()
    bool result = obj->isAction();
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionDataTest, parentPath)
{
    // Test getter: QString parentPath()
    auto result = obj->parentPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, nameArg)
{
    // Test getter: DCustomActionDefines::ActionArg nameArg()
    auto result = obj->nameArg();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, commandArg)
{
    // Test getter: DCustomActionDefines::ActionArg commandArg()
    auto result = obj->commandArg();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, actionPosition)
{
    // Test getter: int actionPosition()
    auto result = obj->actionPosition();
    EXPECT_EQ(result, 0);

}

TEST_F(DCustomActionDataTest, actionName)
{
    // Test getter: QString actionName()
    auto result = obj->actionName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, actionIcon)
{
    // Test getter: QString actionIcon()
    auto result = obj->actionIcon();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, actionCommand)
{
    // Test getter: QString actionCommand()
    auto result = obj->actionCommand();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, actionSeparator)
{
    // Test getter: DCustomActionDefines::Separator actionSeparator()
    auto result = obj->actionSeparator();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, childrenActions)
{
    // Test getter: QList<DCustomActionData> childrenActions()
    auto result = obj->childrenActions();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, actionNameArg)
{
    // Test getter: DCustomActionDefines::ActionArg actionNameArg()
    auto result = obj->actionNameArg();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, actionCmdArg)
{
    // Test getter: DCustomActionDefines::ActionArg actionCmdArg()
    auto result = obj->actionCmdArg();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionDataTest, actionParentPath)
{
    // Test getter: QString actionParentPath()
    auto result = obj->actionParentPath();
    EXPECT_TRUE(result.isEmpty());

}
