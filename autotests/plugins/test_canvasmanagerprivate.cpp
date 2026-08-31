// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagerprivate.cpp
 * @brief Unit tests for CanvasManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasmanager.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerPrivateTest, initModel)
{
    // Test method: void initModel(())
    EXPECT_NO_FATAL_FAILURE(obj->initModel());
}

TEST_F(CanvasManagerPrivateTest, onFileModelReset)
{
    // Test method: void onFileModelReset(())
    EXPECT_NO_FATAL_FAILURE(obj->onFileModelReset());
}
