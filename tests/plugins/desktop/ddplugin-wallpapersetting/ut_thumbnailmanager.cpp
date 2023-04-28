// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "thumbnailmanager.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_WALLPAERSETTING_USE_NAMESPACE

class UT_thumbnailmanager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        tm = new ThumbnailManager(0.5);
    }
    virtual void TearDown() override
    {
        delete tm;
        stub.clear();
    }
    ThumbnailManager *tm = nullptr;
    stub_ext::StubExt stub;
};

TEST(ThumbnailManager, instance)
{
    auto ret = ThumbnailManager::instance(1.0);
    EXPECT_TRUE(qFuzzyCompare(ret->scale, 1.0));
    auto ret2 = ThumbnailManager::instance(0.5);
    auto ret3 = ThumbnailManager::instance(0.5);

    EXPECT_TRUE(qFuzzyCompare(ret2->scale, 0.5));
    EXPECT_NE(ret, ret2);
    EXPECT_EQ(ret2, ret3);

    ret->deleteLater();
    ret2->deleteLater();
    ret3->deleteLater();
}

TEST_F(UT_thumbnailmanager, find)
{
    stub.set_lamda(&QPixmap::isNull, []() {
        return false;
    });
    bool emited = false;
    stub.set_lamda(&ThumbnailManager::thumbnailFounded, [&emited]() {
        emited = true;
    });
    tm->find("test");
    EXPECT_TRUE(emited);

    emited = false;
    EXPECT_TRUE(tm->queuedRequests.isEmpty());
    stub.set_lamda(&QPixmap::isNull, []() {
        return true;
    });
    stub.set_lamda(&ThumbnailManager::processNextReq, []() {
        return;
    });
    tm->find("test");
    EXPECT_FALSE(tm->queuedRequests.isEmpty());
}
