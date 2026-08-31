// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextmenuimplproxyprivate.cpp
 * @brief Unit tests for DFMExtMenuImplProxyPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/dfmextmenuimplproxy.h"

#include <QTest>

using namespace dfmplugin_utils;

class DFMExtMenuImplProxyPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtMenuImplProxyPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtMenuImplProxyPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtMenuImplProxyPrivateTest, deleteAction)
{
    // Test method: bool deleteAction((DFMExtAction *action))
    auto result = obj->deleteAction(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DFMExtMenuImplProxyPrivateTest, deleteMenu)
{
    // Test method: bool deleteMenu((DFMExtMenu *menu))
    auto result = obj->deleteMenu(nullptr);
    EXPECT_FALSE(result);

}
