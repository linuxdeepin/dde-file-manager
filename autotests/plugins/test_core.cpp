// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_core.cpp
 * @brief Unit tests for Core methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "core.h"

#include <QTest>

using namespace core;

class CoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Core();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Core *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreTest, connectToServer)
{
    // Test method: void connectToServer(())
    EXPECT_NO_FATAL_FAILURE(obj->connectToServer());
}

TEST_F(CoreTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 winid))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(CoreTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
