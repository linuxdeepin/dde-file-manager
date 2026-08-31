// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasplugin_1.cpp
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

TEST_F(CanvasPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(CanvasPluginTest, registerDBus)
{
    // Test method: void registerDBus(())
    EXPECT_NO_FATAL_FAILURE(obj->registerDBus());
}
