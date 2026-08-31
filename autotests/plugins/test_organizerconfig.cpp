// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizerconfig.cpp
 * @brief Unit tests for OrganizerConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/organizerconfig.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerConfigTest, collectionBase)
{
    // Test method: CollectionBaseDataPtr collectionBase((bool custom, const QString &key))
    QString _arg1{};
    auto result = obj->collectionBase(false, _arg1);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(OrganizerConfigTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OrganizerConfigTest, surfaceSizes)
{
    // Test getter: QList<QSize> surfaceSizes()
    auto result = obj->surfaceSizes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OrganizerConfigTest, writeCollectionBase)
{
    // Test method: void writeCollectionBase((bool custom, const QList<CollectionBaseDataPtr> &base))
    QList<CollectionBaseDataPtr> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->writeCollectionBase(false, _arg1));
}

TEST_F(OrganizerConfigTest, writeCollectionStyle)
{
    // Test method: void writeCollectionStyle((const QString &styleId, const QList<CollectionStyle> &styles))
    QString _arg0{};
    QList<CollectionStyle> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->writeCollectionStyle(_arg0, _arg1));
}
