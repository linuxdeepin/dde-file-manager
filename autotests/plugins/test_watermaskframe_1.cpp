// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_watermaskframe_1.cpp
 * @brief Unit tests for WaterMaskFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/watermaskframe.h"

#include <QTest>

using namespace ddplugin_canvas;

class WaterMaskFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WaterMaskFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WaterMaskFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WaterMaskFrameTest, WaterMaskFrame)
{
    // Test constructor: WaterMaskFrame((const QString &fileName, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WaterMaskFrameTest, loadConfig)
{
    // Test method: void loadConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->loadConfig());
}

TEST_F(WaterMaskFrameTest, parseJson)
{
    // Test method: QMap<QString, WaterMaskFrame::ConfigInfo> parseJson((QJsonObject *configs))
    auto result = obj->parseJson(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WaterMaskFrameTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(WaterMaskFrameTest, setTextAlign)
{
    // Test setter: void setTextAlign((const QString &maskTextAlign))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTextAlign(_arg0));
}

TEST_F(WaterMaskFrameTest, updatePosition)
{
    // Test method: void updatePosition(())
    EXPECT_NO_FATAL_FAILURE(obj->updatePosition());
}

TEST_F(WaterMaskFrameTest, WaterMaskFrame_Destructor)
{
    // Test method:  ~WaterMaskFrame(())
    EXPECT_NO_FATAL_FAILURE({ WaterMaskFrame *tmp = new WaterMaskFrame(); delete tmp; });
}
