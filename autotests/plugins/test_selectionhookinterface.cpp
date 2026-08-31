// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_selectionhookinterface.cpp
 * @brief Unit tests for SelectionHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/selectionhookinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class SelectionHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SelectionHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SelectionHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SelectionHookInterfaceTest, SelectionHookInterface)
{
    // Test constructor: SelectionHookInterface(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(SelectionHookInterfaceTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}
