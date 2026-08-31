// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowframeprivate.cpp
 * @brief Unit tests for WindowFramePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "frame/windowframe.h"

#include <QTest>

using namespace ddplugin_core;

class WindowFramePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WindowFramePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WindowFramePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WindowFramePrivateTest, createWindow)
{
    // Test method: BaseWindowPointer createWindow((ScreenPointer sp))
    auto result = obj->createWindow(ScreenPointer());
    EXPECT_NE(result.get(), nullptr);

}
