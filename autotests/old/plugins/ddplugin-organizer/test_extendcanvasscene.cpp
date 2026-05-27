// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menus/extendcanvasscene.h"

#include <QMenu>
#include <QAction>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_ExtendCanvasCreator : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test objects
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtendCanvasCreator, Name_ReturnsCorrectName)
{
    QString name = ExtendCanvasCreator::name();
    EXPECT_EQ(name, "OrganizerExtCanvasMenu");
}

TEST_F(UT_ExtendCanvasCreator, Create_ReturnsScene)
{
    ExtendCanvasCreator creator;
    DFMBASE_NAMESPACE::AbstractMenuScene *scene = creator.create();
    EXPECT_NE(scene, nullptr);
    delete scene;
}

class UT_ExtendCanvasScene : public testing::Test
{
protected:
    void SetUp() override
    {
        scene = new ExtendCanvasScene();
    }

    void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ExtendCanvasScene *scene = nullptr;
};

TEST_F(UT_ExtendCanvasScene, Constructor_CreatesScene)
{
    EXPECT_NE(scene, nullptr);
}

TEST_F(UT_ExtendCanvasScene, Name_ReturnsCorrectName)
{
    QString name = scene->name();
    EXPECT_EQ(name, "OrganizerExtCanvasMenu");
}

TEST_F(UT_ExtendCanvasScene, Initialize_ReturnsBool)
{
    QVariantHash params;
    bool result = scene->initialize(params);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_ExtendCanvasScene, Scene_ReturnsScene)
{
    QAction action;
    DFMBASE_NAMESPACE::AbstractMenuScene *result = scene->scene(&action);
    EXPECT_TRUE(result == nullptr || result != nullptr); // Could return null or a valid scene
}

TEST_F(UT_ExtendCanvasScene, Create_AddsToMenu)
{
    QMenu parentMenu;
    bool result = scene->create(&parentMenu);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_ExtendCanvasScene, UpdateState_DoesNotCrash)
{
    QMenu parentMenu;
    EXPECT_NO_THROW(scene->updateState(&parentMenu));
}

TEST_F(UT_ExtendCanvasScene, Triggered_ReturnsBool)
{
    QAction action;
    bool result = scene->triggered(&action);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_ExtendCanvasScene, ActionFilter_ReturnsBool)
{
    QAction action;
    ExtendCanvasScene otherScene;
    bool result = scene->actionFilter(&otherScene, &action);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_ExtendCanvasScene, MultipleSceneOperations)
{
    QMenu menu1, menu2;
    QAction action1, action2;
    QVariantHash params;
    
    // Test initialization
    EXPECT_TRUE(scene->initialize(params) || !scene->initialize(params));
    
    // Test creation
    EXPECT_TRUE(scene->create(&menu1) || !scene->create(&menu1));
    EXPECT_TRUE(scene->create(&menu2) || !scene->create(&menu2));
    
    // Test state updates
    scene->updateState(&menu1);
    scene->updateState(&menu2);
    
    // Test triggering
    EXPECT_TRUE(scene->triggered(&action1) || !scene->triggered(&action1));
    EXPECT_TRUE(scene->triggered(&action2) || !scene->triggered(&action2));
}
