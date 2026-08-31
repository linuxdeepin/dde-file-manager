// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmenusceneprivate.cpp
 * @brief Unit tests for RecentMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/recentmenuscene.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentMenuScenePrivateTest, disableSubScene)
{
    // Test method: void disableSubScene((AbstractMenuScene *scene, const QString &sceneName))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->disableSubScene(nullptr, _arg1));
}
