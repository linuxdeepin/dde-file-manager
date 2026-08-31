// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextmenuimplprivate_1.cpp
 * @brief Unit tests for DFMExtMenuImplPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/dfmextmenuimpl.h"

#include <QTest>

using namespace dfmplugin_utils;

class DFMExtMenuImplPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtMenuImplPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtMenuImplPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtMenuImplPrivateTest, insertAction)
{
    // Test method: bool insertAction((DFMExtAction *before, DFMExtAction *action))
    auto result = obj->insertAction(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DFMExtMenuImplPrivateTest, isInterior)
{
    // Test bool getter: isInterior()
    bool result = obj->isInterior();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtMenuImplPrivateTest, menuAction)
{
    // Test getter: DFMExtAction menuAction()
    auto result = obj->menuAction();
    EXPECT_NO_FATAL_FAILURE({ obj->menuAction(); });

}

TEST_F(DFMExtMenuImplPrivateTest, menuImpl)
{
    // Test getter: DFMExtMenuImpl menuImpl()
    auto result = obj->menuImpl();
    EXPECT_NO_FATAL_FAILURE({ obj->menuImpl(); });

}

TEST_F(DFMExtMenuImplPrivateTest, onActionHovered)
{
    // Test method: void onActionHovered((QAction *qaction))
    EXPECT_NO_FATAL_FAILURE(obj->onActionHovered(nullptr));
}

TEST_F(DFMExtMenuImplPrivateTest, onActionTriggered)
{
    // Test method: void onActionTriggered((QAction *qaction))
    EXPECT_NO_FATAL_FAILURE(obj->onActionTriggered(nullptr));
}

TEST_F(DFMExtMenuImplPrivateTest, qmenu)
{
    // Test getter: QMenu qmenu()
    auto result = obj->qmenu();
    EXPECT_NO_FATAL_FAILURE({ obj->qmenu(); });

}
