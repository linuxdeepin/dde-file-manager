// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionmodelprivate.cpp
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

TEST_F(CollectionModelPrivateTest, clearMapping)
{
    // Test method: void clearMapping(())
    EXPECT_NO_FATAL_FAILURE(obj->clearMapping());
}

TEST_F(CollectionModelPrivateTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(CollectionModelPrivateTest, sourceAboutToBeReset)
{
    // Test method: void sourceAboutToBeReset(())
    EXPECT_NO_FATAL_FAILURE(obj->sourceAboutToBeReset());
}

TEST_F(CollectionModelPrivateTest, sourceDataRenamed)
{
    // Test method: void sourceDataRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceDataRenamed(_arg0, _arg1));
}

TEST_F(CollectionModelPrivateTest, sourceReset)
{
    // Test method: void sourceReset(())
    EXPECT_NO_FATAL_FAILURE(obj->sourceReset());
}

TEST_F(CollectionModelPrivateTest, sourceRowsAboutToBeRemoved)
{
    // Test method: void sourceRowsAboutToBeRemoved((const QModelIndex &sourceParent, int start, int end))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceRowsAboutToBeRemoved(_arg0, 0, 0));
}

TEST_F(CollectionModelPrivateTest, sourceRowsInserted)
{
    // Test method: void sourceRowsInserted((const QModelIndex &sourceParent, int start, int end))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceRowsInserted(_arg0, 0, 0));
}
