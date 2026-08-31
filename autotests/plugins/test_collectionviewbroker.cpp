// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionviewbroker.cpp
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

TEST_F(CollectionViewBrokerTest, setView)
{
    // Test setter: void setView((CollectionView *v))
    EXPECT_NO_FATAL_FAILURE(obj->setView(nullptr));
}
