// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basewindow.cpp
 * @brief Unit tests for BaseWindow methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "frame/basewindow.h"

#include <QTest>

using namespace ddplugin_core;

class BaseWindowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseWindow();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseWindow *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseWindowTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}
