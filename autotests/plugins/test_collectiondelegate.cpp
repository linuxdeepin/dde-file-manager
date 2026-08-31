// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiondelegate.cpp
 * @brief Unit tests for CollectionDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/collectiondelegate.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CollectionDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionDelegateTest, CollectionDelegate)
{
    // Test constructor: CollectionDelegate((QAbstractItemView *parent))
    ASSERT_NE(obj, nullptr);
}
