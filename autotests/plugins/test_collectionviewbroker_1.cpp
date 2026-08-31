// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionviewbroker_1.cpp
 * @brief Unit tests for CollectionViewBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/collectionviewbroker.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionViewBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionViewBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewBrokerTest, CollectionViewBroker)
{
    // Test constructor: CollectionViewBroker((CollectionView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionViewBrokerTest, getView)
{
    // Test getter: CollectionView getView()
    auto result = obj->getView();
    EXPECT_NO_FATAL_FAILURE({ obj->getView(); });

}

TEST_F(CollectionViewBrokerTest, gridPoint)
{
    // Test method: bool gridPoint((const QUrl &file, QPoint &pos))
    QUrl _arg0{};
    QPoint _arg1{};
    auto result = obj->gridPoint(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewBrokerTest, visualRect)
{
    // Test method: QRect visualRect((const QUrl &file))
    QUrl _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}
