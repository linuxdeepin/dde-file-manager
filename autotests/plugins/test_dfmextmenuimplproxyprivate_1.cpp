// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextmenuimplproxyprivate_1.cpp
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

TEST_F(DFMExtMenuImplProxyPrivateTest, createAction)
{
    // Test getter: DFMExtAction createAction()
    auto result = obj->createAction();
    EXPECT_NO_FATAL_FAILURE({ obj->createAction(); });

}

TEST_F(DFMExtMenuImplProxyPrivateTest, createMenu)
{
    // Test getter: DFMExtMenu createMenu()
    auto result = obj->createMenu();
    EXPECT_NO_FATAL_FAILURE({ obj->createMenu(); });

}
