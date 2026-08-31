// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredutils.cpp
 * @brief Unit tests for ShredUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredutils.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredUtilsTest, instance)
{
    // Test getter: ShredUtils instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ShredUtilsTest, isValidFile)
{
    // Test method: bool isValidFile((const QUrl &file))
    QUrl _arg0{};
    auto result = obj->isValidFile(_arg0);
    EXPECT_FALSE(result);

}
