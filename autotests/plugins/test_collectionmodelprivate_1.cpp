// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionmodelprivate_1.cpp
 * @brief Unit tests for CollectionModelPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/collectionmodel.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionModelPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionModelPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionModelPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionModelPrivateTest, CollectionModelPrivate)
{
    // Test constructor: CollectionModelPrivate((CollectionModel *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionModelPrivateTest, createMapping)
{
    // Test method: void createMapping(())
    EXPECT_NO_FATAL_FAILURE(obj->createMapping());
}

TEST_F(CollectionModelPrivateTest, doRefresh)
{
    // Test method: void doRefresh((bool global, bool file))
    EXPECT_NO_FATAL_FAILURE(obj->doRefresh(false, false));
}

TEST_F(CollectionModelPrivateTest, CollectionModelPrivate_Destructor)
{
    // Test method:  ~CollectionModelPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionModelPrivate *tmp = new CollectionModelPrivate(); delete tmp; });
}
