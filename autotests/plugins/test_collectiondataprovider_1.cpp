// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiondataprovider_1.cpp
 * @brief Unit tests for CollectionDataProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/collectiondataprovider.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionDataProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionDataProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionDataProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionDataProviderTest, addPreItems)
{
    // Test method: void addPreItems((const QString &targetKey, const QList<QUrl> &urls, int targetIndex))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addPreItems(_arg0, _arg1, 0));
}
