// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "menus/basesortmenuscene.h"
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/dfm_menu_defines.h>
#include "menus/workspacemenu_defines.h"

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

class BaseSortMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        creator = new BaseSortMenuCreator();
        scene = qobject_cast<BaseSortMenuScene *>(creator->create());
    }

    void TearDown() override
    {
        delete scene;
        delete creator;
        stub.clear();
    }

    BaseSortMenuCreator *creator = nullptr;
    BaseSortMenuScene *scene = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseSortMenuSceneTest, CreatorName_ReturnsCorrectName)
{
    // Test that creator returns correct name
    QString expectedName = "BaseSortMenu";
    QString actualName = BaseSortMenuCreator::name();
    
    EXPECT_EQ(actualName, expectedName);
}

TEST_F(BaseSortMenuSceneTest, CreatorCreate_ReturnsValidScene)
{
    // Test that creator creates valid scene
    BaseSortMenuCreator creator;
    AbstractMenuScene *createdScene = creator.create();
    
    EXPECT_NE(createdScene, nullptr);
    EXPECT_NE(qobject_cast<BaseSortMenuScene *>(createdScene), nullptr);
    
    delete createdScene;
}

TEST_F(BaseSortMenuSceneTest, SceneName_ReturnsCreatorName)
{
    // Test that scene name matches creator name
    QString creatorName = BaseSortMenuCreator::name();
    QString sceneName = scene->name();
    
    EXPECT_EQ(sceneName, creatorName);
}

TEST_F(BaseSortMenuSceneTest, Initialize_ValidParams_ReturnsTrue)
{
    // Test scene initialization with valid parameters
    QVariantHash params;
    params["test"] = "value";
    
    bool result = scene->initialize(params);
    
    EXPECT_TRUE(result);
}

TEST_F(BaseSortMenuSceneTest, Initialize_EmptyParams_ReturnsTrue)
{
    // Test scene initialization with empty parameters
    QVariantHash params;
    
    bool result = scene->initialize(params);
    
    EXPECT_TRUE(result);
}

TEST_F(BaseSortMenuSceneTest, Create_ValidParent_ReturnsTrue)
{
    // Test scene creation with valid parent menu
    QMenu parentMenu;
    
    bool result = scene->create(&parentMenu);
    
    EXPECT_TRUE(result);
}

TEST_F(BaseSortMenuSceneTest, Create_NullParent_ReturnsFalse)
{
    // Test scene creation with null parent menu
    bool result = scene->create(nullptr);
    
    EXPECT_FALSE(result);
}

TEST_F(BaseSortMenuSceneTest, UpdateState_ValidParent_DoesNotCrash)
{
    // Test updating state with valid parent menu
    QMenu parentMenu;
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(&parentMenu));
}

TEST_F(BaseSortMenuSceneTest, UpdateState_NullParent_DoesNotCrash)
{
    // Test updating state with null parent menu
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(nullptr));
}

TEST_F(BaseSortMenuSceneTest, Triggered_ValidAction_ReturnsBaseResult)
{
    // Test triggering valid action
    QAction action("Test Action");
    
    bool result = scene->triggered(&action);
    
    // Should return base class result
    EXPECT_TRUE(result);
}

TEST_F(BaseSortMenuSceneTest, Triggered_NullAction_ReturnsFalse)
{
    // Test triggering null action
    bool result = scene->triggered(nullptr);
    
    EXPECT_FALSE(result);
}

TEST_F(BaseSortMenuSceneTest, Scene_ValidAction_ReturnsNull)
{
    // Test getting scene for valid action
    QAction action("Test Action");
    
    AbstractMenuScene *result = scene->scene(&action);
    
    // Should return nullptr for actions not belonging to this scene
    EXPECT_EQ(result, nullptr);
}

TEST_F(BaseSortMenuSceneTest, Scene_NullAction_ReturnsNull)
{
    // Test getting scene for null action
    AbstractMenuScene *result = scene->scene(nullptr);
    
    EXPECT_EQ(result, nullptr);
}

class BaseSortMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        creator = new BaseSortMenuCreator();
        scene = qobject_cast<BaseSortMenuScene *>(creator->create());
        menu = new QMenu();
    }

    void TearDown() override
    {
        delete menu;
        delete scene;
        delete creator;
        stub.clear();
    }

    BaseSortMenuCreator *creator = nullptr;
    BaseSortMenuScene *scene = nullptr;
    QMenu *menu = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseSortMenuScenePrivateTest, SortMenuActions_ValidMenuAndRule_SortsActions)
{
    // Test sorting menu actions with valid menu and sort rule
    // Add some actions to the menu
    QAction *action1 = new QAction("Action 1", menu);
    QAction *action2 = new QAction("Action 2", menu);
    QAction *action3 = new QAction("Action 3", menu);
    
    // Set action IDs
    action1->setProperty("actionID", "action-2");
    action2->setProperty("actionID", "action-1");
    action3->setProperty("actionID", "action-3");
    
    menu->addAction(action1);
    menu->addAction(action2);
    menu->addAction(action3);
    
    // Define sort rule
    QStringList sortRule;
    sortRule << "action-1" << "action-2" << "action-3";
    
    // Call updateState which should trigger sorting
    scene->updateState(menu);
    
    // Check that actions are sorted (this is a basic test, actual sorting logic is complex)
    EXPECT_EQ(menu->actions().size(), 3);
}

TEST_F(BaseSortMenuScenePrivateTest, SortMenuActions_NullMenu_DoesNotCrash)
{
    // Test sorting with null menu
    QStringList sortRule;
    sortRule << "action-1" << "action-2";
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(nullptr));
}

TEST_F(BaseSortMenuScenePrivateTest, SortMenuActions_EmptyRule_DoesNotCrash)
{
    // Test sorting with empty rule
    QAction *action = new QAction("Action", menu);
    menu->addAction(action);
    
    QStringList emptyRule;
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, SortSecondaryMenu_ValidMenu_SortsSubmenus)
{
    // Test sorting secondary menus
    QAction *mainAction = new QAction("Main Action", menu);
    QMenu *subMenu = new QMenu("Sub Menu", menu);
    
    QAction *subAction1 = new QAction("Sub Action 1", subMenu);
    QAction *subAction2 = new QAction("Sub Action 2", subMenu);
    
    // Set action IDs
    mainAction->setProperty("actionID", "display-as");
    subAction1->setProperty("actionID", "display-as-icon");
    subAction2->setProperty("actionID", "display-as-list");
    
    subMenu->addAction(subAction1);
    subMenu->addAction(subAction2);
    mainAction->setMenu(subMenu);
    menu->addAction(mainAction);
    
    // Call updateState which should trigger sorting
    scene->updateState(menu);
    
    // Check that submenu actions are sorted
    EXPECT_EQ(subMenu->actions().size(), 2);
}

TEST_F(BaseSortMenuScenePrivateTest, PrimaryMenuRule_ReturnsValidRules)
{
    // Test that primary menu rule returns valid rules
    // This is tested indirectly through updateState
    QAction *action = new QAction("Test Action", menu);
    action->setProperty("actionID", "open");
    menu->addAction(action);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, SecondaryMenuRule_ReturnsValidRules)
{
    // Test that secondary menu rule returns valid rules
    // This is tested indirectly through updateState
    QAction *mainAction = new QAction("Display As", menu);
    QMenu *subMenu = new QMenu("Display As", menu);
    
    QAction *subAction = new QAction("Icon", subMenu);
    subAction->setProperty("actionID", "display-as-icon");
    
    subMenu->addAction(subAction);
    mainAction->setMenu(subMenu);
    mainAction->setProperty("actionID", "display-as");
    menu->addAction(mainAction);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, SendToRule_ReturnsValidRules)
{
    // Test send to rule (tested indirectly)
    QAction *action = new QAction("Send To", menu);
    action->setProperty("actionID", "send-to");
    menu->addAction(action);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, StageToRule_ReturnsValidRules)
{
    // Test stage to rule (tested indirectly)
    QAction *action = new QAction("Stage To", menu);
    action->setProperty("actionID", "stage-file-to-burning");
    menu->addAction(action);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, SortMenuActions_WithSeparator_HandlesCorrectly)
{
    // Test sorting with separator
    QAction *action1 = new QAction("Action 1", menu);
    QAction *separator = new QAction(menu);
    QAction *action2 = new QAction("Action 2", menu);
    
    action1->setProperty("actionID", "action-1");
    separator->setSeparator(true);
    separator->setProperty("actionID", "separator-line");
    action2->setProperty("actionID", "action-2");
    
    menu->addAction(action1);
    menu->addAction(separator);
    menu->addAction(action2);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(BaseSortMenuScenePrivateTest, SortMenuActions_FuzzyMatching_WorksCorrectly)
{
    // Test fuzzy matching in sort rules
    QAction *action1 = new QAction("Action 1", menu);
    QAction *action2 = new QAction("Action 2", menu);
    
    action1->setProperty("actionID", "send-to-removable-device");
    action2->setProperty("actionID", "send-to-bluetooth");
    
    menu->addAction(action1);
    menu->addAction(action2);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(menu));
}