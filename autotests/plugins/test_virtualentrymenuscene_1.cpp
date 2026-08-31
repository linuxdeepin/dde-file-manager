// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrymenuscene_1.cpp
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

TEST_F(VirtualEntryMenuSceneTest, VirtualEntryMenuScene)
{
    // Test constructor: VirtualEntryMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VirtualEntryMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VirtualEntryMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(VirtualEntryMenuSceneTest, VirtualEntryMenuScene_Destructor)
{
    // Test method:  ~VirtualEntryMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ VirtualEntryMenuScene *tmp = new VirtualEntryMenuScene(); delete tmp; });
}
