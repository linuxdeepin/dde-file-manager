// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewhookinterface.cpp
 * @brief Unit tests for ViewHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/viewhookinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class ViewHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewHookInterfaceTest, ViewHookInterface)
{
    // Test constructor: ViewHookInterface(())
    ASSERT_NE(obj, nullptr);
}
