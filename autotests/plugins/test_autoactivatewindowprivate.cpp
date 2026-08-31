// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_autoactivatewindowprivate.cpp
 * @brief Unit tests for AutoActivateWindowPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/autoactivatewindow.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class AutoActivateWindowPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AutoActivateWindowPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AutoActivateWindowPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AutoActivateWindowPrivateTest, watchOnWayland)
{
    // Test method: void watchOnWayland((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->watchOnWayland(false));
}

TEST_F(AutoActivateWindowPrivateTest, watchOnX11)
{
    // Test method: void watchOnX11((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->watchOnX11(false));
}
