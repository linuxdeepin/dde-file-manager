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

TEST_F(UT_thumbnailmanager, stop)
{
    tm->queuedRequests.append("test");
    EXPECT_FALSE(tm->queuedRequests.isEmpty());

    EXPECT_NO_FATAL_FAILURE(tm->stop());

    EXPECT_TRUE(tm->futureWatcher.isCanceled());
    EXPECT_TRUE(tm->queuedRequests.isEmpty());
}

TEST_F(UT_thumbnailmanager, onProcessFinished)
{
    auto iscanceledFunc = static_cast<bool (QFutureWatcher<QPixmap>::*)() const>(&QFutureWatcher<QPixmap>::isCanceled);
    stub.set_lamda(iscanceledFunc, []() {
        __DBG_STUB_INVOKE__
        return false;
    });
    auto resultFunc = static_cast<QPixmap (QFutureWatcher<QPixmap>::*)() const>(&QFutureWatcher<QPixmap>::result);
    stub.set_lamda(resultFunc, []() {
        __DBG_STUB_INVOKE__
        return QPixmap();
    });

    bool isEmit = false;
    bool call = false;
    stub.set_lamda(&ThumbnailManager::thumbnailFounded, [&isEmit]() {
        __DBG_STUB_INVOKE__
        isEmit = true;
    });

    {
        tm->queuedRequests.append("test");
        stub.set_lamda(&ddplugin_wallpapersetting::ThumbnailManager::processNextReq, [&call]() {
            call = true;
        });

        tm->onProcessFinished();
        EXPECT_TRUE(tm->queuedRequests.isEmpty());
        EXPECT_TRUE(isEmit);
        EXPECT_FALSE(call);
    }
    {
        isEmit = false;
        call = false;
        tm->queuedRequests.append("test1");
        tm->queuedRequests.append("test2");
        tm->onProcessFinished();
        EXPECT_FALSE(tm->queuedRequests.isEmpty());
        EXPECT_TRUE(isEmit);
        EXPECT_TRUE(call);
    }
    {
        stub.set_lamda(iscanceledFunc, []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        isEmit = false;
        call = false;
        tm->onProcessFinished();
        EXPECT_FALSE(isEmit);
        EXPECT_FALSE(call);
    }
}

TEST_F(UT_thumbnailmanager, processNextReq)
{
    auto isFinishedFunc = static_cast<bool (QFutureWatcher<QPixmap>::*)() const>(&QFutureWatcher<QPixmap>::isFinished);
    stub.set_lamda(isFinishedFunc, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    bool call = false;
    QString input;
    stub.set_lamda(&ThumbnailManager::thumbnailImage, [&call, &input](const QString &key, qreal scale) {
        Q_UNUSED(scale);
        call = true;
        input = key;
        __DBG_STUB_INVOKE__
        return QPixmap();
    });

    tm->queuedRequests.append("test1");
    tm->processNextReq();
    usleep(100);
    EXPECT_TRUE(call);
    EXPECT_EQ(input, QString("test1"));
}
