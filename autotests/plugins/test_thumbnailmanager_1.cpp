// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailmanager_1.cpp
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

TEST_F(ThumbnailManagerTest, ThumbnailManager)
{
    // Test constructor: ThumbnailManager((qreal _scale, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ThumbnailManagerTest, onProcessFinished)
{
    // Test method: void onProcessFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onProcessFinished());
}

TEST_F(ThumbnailManagerTest, processNextReq)
{
    // Test method: void processNextReq(())
    EXPECT_NO_FATAL_FAILURE(obj->processNextReq());
}

TEST_F(ThumbnailManagerTest, replace)
{
    // Test method: bool replace((const QString &key, const QPixmap &pixmap))
    QString _arg0{};
    QPixmap _arg1{};
    auto result = obj->replace(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ThumbnailManagerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(ThumbnailManagerTest, ThumbnailManager_Destructor)
{
    // Test method:  ~ThumbnailManager(())
    EXPECT_NO_FATAL_FAILURE({ ThumbnailManager *tmp = new ThumbnailManager(); delete tmp; });
}
