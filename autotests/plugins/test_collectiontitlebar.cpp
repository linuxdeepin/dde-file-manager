// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiontitlebar.cpp
 * @brief Unit tests for CollectionTitleBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectiontitlebar.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionTitleBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionTitleBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionTitleBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionTitleBarTest, adjustable)
{
    // Test bool getter: adjustable()
    bool result = obj->adjustable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionTitleBarTest, closable)
{
    // Test bool getter: closable()
    bool result = obj->closable();
    EXPECT_FALSE(result);

}
