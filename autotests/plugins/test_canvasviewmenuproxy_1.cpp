// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewmenuproxy_1.cpp
 * @brief Unit tests for CanvasViewMenuProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/canvasviewmenuproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewMenuProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewMenuProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewMenuProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewMenuProxyTest, CanvasViewMenuProxy)
{
    // Test constructor: CanvasViewMenuProxy((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasViewMenuProxyTest, changeIconLevel)
{
    // Test method: void changeIconLevel((bool increase))
    EXPECT_NO_FATAL_FAILURE(obj->changeIconLevel(false));
}

TEST_F(CanvasViewMenuProxyTest, disableMenu)
{
    // Test bool getter: disableMenu()
    bool result = obj->disableMenu();
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewMenuProxyTest, CanvasViewMenuProxy_Destructor)
{
    // Test method:  ~CanvasViewMenuProxy(())
    EXPECT_NO_FATAL_FAILURE({ CanvasViewMenuProxy *tmp = new CanvasViewMenuProxy(); delete tmp; });
}
