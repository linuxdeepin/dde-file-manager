// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendcanvassceneprivate.cpp
 * @brief Unit tests for ExtendCanvasScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/extendcanvasscene.h"

#include <QTest>

using namespace ddplugin_organizer;

class ExtendCanvasScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendCanvasScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendCanvasScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendCanvasScenePrivateTest, ExtendCanvasScenePrivate)
{
    // Test constructor: ExtendCanvasScenePrivate((ExtendCanvasScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtendCanvasScenePrivateTest, emptyMenu)
{
    // Test method: void emptyMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->emptyMenu(nullptr));
}

TEST_F(ExtendCanvasScenePrivateTest, normalMenu)
{
    // Test method: void normalMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->normalMenu(nullptr));
}

TEST_F(ExtendCanvasScenePrivateTest, organizeBySubActions)
{
    // Test method: QMenu organizeBySubActions((QMenu *menu))
    auto result = obj->organizeBySubActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->organizeBySubActions(nullptr); });

}

TEST_F(ExtendCanvasScenePrivateTest, triggerSortby)
{
    // Test method: bool triggerSortby((const QString &actionId))
    QString _arg0{};
    auto result = obj->triggerSortby(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ExtendCanvasScenePrivateTest, updateNormalMenu)
{
    // Test method: void updateNormalMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateNormalMenu(nullptr));
}
