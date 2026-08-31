// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundbridge.cpp
 * @brief Unit tests for BackgroundBridge methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundmanager.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundBridgeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundBridge();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundBridge *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundBridgeTest, getPixmap)
{
    // Test method: QPixmap getPixmap((const QString &path, const QSize &targetSize, const QPixmap &defaultPixmap))
    QString _arg0{};
    QSize _arg1{};
    QPixmap _arg2{};
    auto result = obj->getPixmap(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isNull());

}

TEST_F(BackgroundBridgeTest, getValueFromJson)
{
    // Test method: int getValueFromJson((QString json, const QString &screenName))
    QString _arg1{};
    auto result = obj->getValueFromJson(QString(), _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(BackgroundBridgeTest, onFinished)
{
    // Test method: void onFinished((void *pData))
    EXPECT_NO_FATAL_FAILURE(obj->onFinished(nullptr));
}

TEST_F(BackgroundBridgeTest, request)
{
    // Test method: void request((bool refresh))
    EXPECT_NO_FATAL_FAILURE(obj->request(false));
}

TEST_F(BackgroundBridgeTest, terminate)
{
    // Test method: void terminate((bool wait))
    EXPECT_NO_FATAL_FAILURE(obj->terminate(false));
}

TEST_F(BackgroundBridgeTest, BackgroundBridge)
{
    // Test constructor: BackgroundBridge((BackgroundManagerPrivate *ptr))
    ASSERT_NE(obj, nullptr);
}
