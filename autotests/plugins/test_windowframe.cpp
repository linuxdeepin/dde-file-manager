// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowframe.cpp
 * @brief Unit tests for WindowFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "frame/windowframe.h"

#include <QTest>

using namespace ddplugin_core;

class WindowFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WindowFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WindowFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WindowFrameTest, rootWindows)
{
    // Test getter: QList<QWidget *> rootWindows()
    auto result = obj->rootWindows();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}
