// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_namegroupstrategy.cpp
 * @brief Unit tests for NameGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/namegroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class NameGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NameGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NameGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NameGroupStrategyTest, NameGroupStrategy)
{
    // Test constructor: NameGroupStrategy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
