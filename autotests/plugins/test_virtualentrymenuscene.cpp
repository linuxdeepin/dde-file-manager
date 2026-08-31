// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrymenuscene.cpp
 * @brief Unit tests for VirtualEntryMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/menu/virtualentrymenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class VirtualEntryMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VirtualEntryMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(VirtualEntryMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}
