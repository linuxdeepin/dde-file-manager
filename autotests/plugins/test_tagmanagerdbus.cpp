// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmanagerdbus.cpp
 * @brief Unit tests for TagManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tagmanagerdbus.h"

#include <QTest>

using namespace tag;

class TagManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagManagerDBusTest, Delete)
{
    // Test method: bool Delete((int opt, const QVariantMap value))
    auto result = obj->Delete(0, QVariantMap());
    EXPECT_FALSE(result);

}
