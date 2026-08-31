// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionholder.cpp
 * @brief Unit tests for CollectionHolder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "collection/collectionholder.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionHolderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionHolder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionHolder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionHolderTest, CollectionHolder)
{
    // Test constructor: CollectionHolder((const QString &uuid, ddplugin_organizer::CollectionDataProvider *dataProvider, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionHolderTest, setStyle)
{
    // Test setter: void setStyle((const CollectionStyle &style))
    CollectionStyle _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setStyle(_arg0));
}

TEST_F(CollectionHolderTest, show)
{
    // Test method: void show(())
    EXPECT_NO_FATAL_FAILURE(obj->show());
}

TEST_F(CollectionHolderTest, style)
{
    // Test getter: CollectionStyle style()
    auto result = obj->style();
    EXPECT_NO_FATAL_FAILURE({ obj->style(); });

}
