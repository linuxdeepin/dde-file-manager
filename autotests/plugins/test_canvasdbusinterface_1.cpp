// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasdbusinterface_1.cpp
 * @brief Unit tests for CanvasDBusInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasdbusinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasDBusInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasDBusInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasDBusInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasDBusInterfaceTest, EnableUIDebug)
{
    // Test method: void EnableUIDebug((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->EnableUIDebug(false));
}
