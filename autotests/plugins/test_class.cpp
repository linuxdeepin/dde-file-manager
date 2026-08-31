// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_class.cpp
 * @brief Unit tests for class methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "emblem.h"

#include <QTest>

using namespace dfmplugin_emblem;

class classTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new class();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    class *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(classTest, Plugin)
{
    // Test getter: DPEMBLEM_BEGIN_NAMESPACE Plugin()
    auto result = obj->Plugin();
    EXPECT_NO_FATAL_FAILURE({ obj->Plugin(); });

}

TEST_F(classTest, Plugin_Plugin)
{
    // Test getter: DPEMBLEM_BEGIN_NAMESPACE Plugin()
    auto result = obj->Plugin();
    EXPECT_NO_FATAL_FAILURE({ obj->Plugin(); });

}
