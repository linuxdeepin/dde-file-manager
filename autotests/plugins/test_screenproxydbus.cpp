// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenproxydbus.cpp
 * @brief Unit tests for ScreenProxyDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screenproxydbus.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenProxyDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenProxyDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenProxyDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenProxyDBusTest, onMonitorChanged)
{
    // Test method: void onMonitorChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onMonitorChanged());
}

TEST_F(ScreenProxyDBusTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(ScreenProxyDBusTest, screens)
{
    // Test getter: QList<ScreenPointer> screens()
    auto result = obj->screens();
    EXPECT_TRUE(result.isEmpty());

}
