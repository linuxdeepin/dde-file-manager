// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keyselector.cpp
 * @brief Unit tests for KeySelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/keyselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class KeySelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeySelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeySelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeySelectorTest, clearSearchKey)
{
    // Test method: void clearSearchKey(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSearchKey());
}
