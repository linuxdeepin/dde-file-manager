// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasplugin.cpp
 * @brief Unit tests for CanvasPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasplugin.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(CanvasPluginTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
