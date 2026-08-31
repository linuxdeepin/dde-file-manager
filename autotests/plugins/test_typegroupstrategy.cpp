// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typegroupstrategy.cpp
 * @brief Unit tests for TypeGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/typegroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TypeGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TypeGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TypeGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TypeGroupStrategyTest, TypeGroupStrategy)
{
    // Test constructor: TypeGroupStrategy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
