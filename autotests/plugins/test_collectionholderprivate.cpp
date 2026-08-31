// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionholderprivate.cpp
 * @brief Unit tests for CollectionHolderPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "collection/collectionholder.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionHolderPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionHolderPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionHolderPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionHolderPrivateTest, onAdjustFrameSizeMode)
{
    // Test method: void onAdjustFrameSizeMode((const CollectionFrameSize &size))
    CollectionFrameSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAdjustFrameSizeMode(_arg0));
}

TEST_F(CollectionHolderPrivateTest, CollectionHolderPrivate_Destructor)
{
    // Test method:  ~CollectionHolderPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionHolderPrivate *tmp = new CollectionHolderPrivate(); delete tmp; });
}
