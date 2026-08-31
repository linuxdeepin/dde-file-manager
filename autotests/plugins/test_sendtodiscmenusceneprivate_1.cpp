// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtodiscmenusceneprivate_1.cpp
 * @brief Unit tests for SendToDiscMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sendtodiscmenuscene.h"

#include <QTest>

using namespace dfmplugin_burn;

class SendToDiscMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToDiscMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToDiscMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToDiscMenuScenePrivateTest, SendToDiscMenuScenePrivate)
{
    // Test constructor: SendToDiscMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SendToDiscMenuScenePrivateTest, actionMountImage)
{
    // Test method: void actionMountImage(())
    EXPECT_NO_FATAL_FAILURE(obj->actionMountImage());
}

TEST_F(SendToDiscMenuScenePrivateTest, actionPacketWriting)
{
    // Test method: void actionPacketWriting((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->actionPacketWriting(_arg0));
}

TEST_F(SendToDiscMenuScenePrivateTest, actionStageFileForBurning)
{
    // Test method: void actionStageFileForBurning((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->actionStageFileForBurning(_arg0));
}

TEST_F(SendToDiscMenuScenePrivateTest, addSubStageActions)
{
    // Test method: void addSubStageActions((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->addSubStageActions(nullptr));
}

TEST_F(SendToDiscMenuScenePrivateTest, disbaleWoringDevAction)
{
    // Test method: bool disbaleWoringDevAction((QAction *act))
    auto result = obj->disbaleWoringDevAction(nullptr);
    EXPECT_FALSE(result);

}
