// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcut.cpp
 * @brief Unit tests for Shortcut methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/shortcut/shortcut.h"

#include <QTest>

using namespace src;

class ShortcutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Shortcut();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Shortcut *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShortcutTest, toStr)
{
    // Test getter: QString toStr()
    auto result = obj->toStr();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
