// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailmanager.cpp
 * @brief Unit tests for ThumbnailManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "thumbnailmanager.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class ThumbnailManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ThumbnailManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ThumbnailManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ThumbnailManagerTest, find)
{
    // Test method: void find((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->find(_arg0));
}

TEST_F(ThumbnailManagerTest, instance)
{
    // Test method: ThumbnailManager instance((qreal scale))
    auto result = obj->instance(0.0);
    EXPECT_NO_FATAL_FAILURE({ obj->instance(0.0); });

}

TEST_F(ThumbnailManagerTest, thumbnailImage)
{
    // Test method: QPixmap thumbnailImage((const QString &key, qreal scale))
    QString _arg0{};
    auto result = obj->thumbnailImage(_arg0, 0.0);
    EXPECT_TRUE(result.isNull());

}
