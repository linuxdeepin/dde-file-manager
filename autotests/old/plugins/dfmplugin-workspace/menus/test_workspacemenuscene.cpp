// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "menus/workspacemenuscene.h"
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/dfm_menu_defines.h>
#include "menus/workspacemenu_defines.h"
#include "views/fileview.h"

#include <QMenu>
#include <QAction>
#include <QVariantHash>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

class WorkspaceMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        creator = new WorkspaceMenuCreator();
        scene = qobject_cast<WorkspaceMenuScene *>(creator->create());
    }

    void TearDown() override
    {
        delete scene;
        delete creator;
        stub.clear();
    }

    WorkspaceMenuCreator *creator = nullptr;
    WorkspaceMenuScene *scene = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceMenuSceneTest, CreatorName_ReturnsCorrectName)
{
    // Test that creator returns correct name
    QString expectedName = "WorkspaceMenu";
    QString actualName = WorkspaceMenuCreator::name();
    
    EXPECT_EQ(actualName, expectedName);
}

TEST_F(WorkspaceMenuSceneTest, CreatorCreate_ReturnsValidScene)
{
    // Test that creator creates valid scene
    WorkspaceMenuCreator creator;
    AbstractMenuScene *createdScene = creator.create();
    
    EXPECT_NE(createdScene, nullptr);
    EXPECT_NE(qobject_cast<WorkspaceMenuScene *>(createdScene), nullptr);
    
    delete createdScene;
}

TEST_F(WorkspaceMenuSceneTest, SceneName_ReturnsCreatorName)
{
    // Test that scene name matches creator name
    QString creatorName = WorkspaceMenuCreator::name();
    QString sceneName = scene->name();
    
    EXPECT_EQ(sceneName, creatorName);
}

TEST_F(WorkspaceMenuSceneTest, Initialize_ValidParams_ReturnsTrue)
{
    // Test scene initialization with valid parameters
    QVariantHash params;
    params["test"] = "value";
    
    bool result = scene->initialize(params);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTest, Initialize_EmptyParams_ReturnsTrue)
{
    // Test scene initialization with empty parameters
    QVariantHash params;
    
    bool result = scene->initialize(params);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTest, Create_ValidParent_ReturnsTrue)
{
    // Test scene creation with valid parent menu
    // Use stub to avoid Q_ASSERT failure due to null view
    QMenu parentMenu;
    
    // Stub the qobject_cast to return a valid pointer
    using QObjCastFunc = FileView* (*)(QObject*);
    stub.set_lamda((QObjCastFunc)qobject_cast<FileView*>, [](QObject *obj) -> FileView* {
        Q_UNUSED(obj);
        // Return a mock FileView pointer to avoid Q_ASSERT
        static int mockView = 0;
        return reinterpret_cast<FileView*>(&mockView);
    });
    
    bool result = scene->create(&parentMenu);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTest, Create_NullParent_ReturnsFalse)
{
    // Test scene creation with null parent menu
    bool result = scene->create(nullptr);
    
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceMenuSceneTest, UpdateState_ValidParent_DoesNotCrash)
{
    // Test updating state with valid parent menu
    QMenu parentMenu;
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(&parentMenu));
}

TEST_F(WorkspaceMenuSceneTest, UpdateState_NullParent_DoesNotCrash)
{
    // Test updating state with null parent menu
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(nullptr));
}

TEST_F(WorkspaceMenuSceneTest, Scene_ValidAction_ReturnsNull)
{
    // Test getting scene for valid action
    QAction action("Test Action");
    
    AbstractMenuScene *result = scene->scene(&action);
    
    // Should return nullptr for actions not belonging to this scene
    EXPECT_EQ(result, nullptr);
}

TEST_F(WorkspaceMenuSceneTest, Scene_NullAction_ReturnsNull)
{
    // Test getting scene for null action
    AbstractMenuScene *result = scene->scene(nullptr);
    
    EXPECT_EQ(result, nullptr);
}

TEST_F(WorkspaceMenuSceneTest, Triggered_ValidAction_ReturnsBaseResult)
{
    // Test triggering valid action
    QAction action("Test Action");
    
    bool result = scene->triggered(&action);
    
    // Should return base class result
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTest, Triggered_NullAction_ReturnsFalse)
{
    // Test triggering null action
    bool result = scene->triggered(nullptr);
    
    EXPECT_FALSE(result);
}

class WorkspaceMenuSceneTriggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        creator = new WorkspaceMenuCreator();
        scene = qobject_cast<WorkspaceMenuScene *>(creator->create());
    }

    void TearDown() override
    {
        delete scene;
        delete creator;
        stub.clear();
    }

    WorkspaceMenuCreator *creator = nullptr;
    WorkspaceMenuScene *scene = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceMenuSceneTriggerTest, EmptyMenuTriggered_ValidAction_ReturnsBaseResult)
{
    // Test empty menu triggered with valid action
    QAction action("Test Action");
    
    // Use reflection to call private method
    bool result = false;
    QMetaObject::invokeMethod(scene, "emptyMenuTriggered", Qt::DirectConnection,
                           Q_RETURN_ARG(bool, result),
                           Q_ARG(QAction*, &action));
    
    // Should return base class result
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, EmptyMenuTriggered_NullAction_ReturnsFalse)
{
    // Test empty menu triggered with null action
    bool result = false;
    QMetaObject::invokeMethod(scene, "emptyMenuTriggered", Qt::DirectConnection,
                           Q_RETURN_ARG(bool, result),
                           Q_ARG(QAction*, nullptr));
    
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, NormalMenuTriggered_ValidAction_ReturnsBaseResult)
{
    // Test normal menu triggered with valid action
    QAction action("Test Action");
    
    // Use reflection to call private method
    bool result = false;
    QMetaObject::invokeMethod(scene, "normalMenuTriggered", Qt::DirectConnection,
                           Q_RETURN_ARG(bool, result),
                           Q_ARG(QAction*, &action));
    
    // Should return base class result
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, NormalMenuTriggered_NullAction_ReturnsFalse)
{
    // Test normal menu triggered with null action
    bool result = false;
    QMetaObject::invokeMethod(scene, "normalMenuTriggered", Qt::DirectConnection,
                           Q_RETURN_ARG(bool, result),
                           Q_ARG(QAction*, nullptr));
    
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithEmptyAreaParams_HandlesCorrectly)
{
    // Test triggered with empty area parameters
    QVariantHash params;
    params[MenuParamKey::kIsEmptyArea] = true;
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithNonEmptyAreaParams_HandlesCorrectly)
{
    // Test triggered with non-empty area parameters
    QVariantHash params;
    params[MenuParamKey::kIsEmptyArea] = false;
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithSelectedUrlsParams_HandlesCorrectly)
{
    // Test triggered with selected URLs parameters
    QVariantHash params;
    params[MenuParamKey::kIsEmptyArea] = false;
    
    QList<QUrl> selectedUrls;
    selectedUrls << QUrl("file:///test1") << QUrl("file:///test2");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectedUrls);
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithWindowIdParams_HandlesCorrectly)
{
    // Test triggered with window ID parameters
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithDirectoryParams_HandlesCorrectly)
{
    // Test triggered with directory parameters
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("file:///test");
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithSelectFilesParams_HandlesCorrectly)
{
    // Test triggered with select files parameters (focus file)
    QVariantHash params;
    QList<QUrl> selectFiles;
    selectFiles << QUrl("file:///test.txt");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectFiles);
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithMultipleParams_HandlesCorrectly)
{
    // Test triggered with multiple parameters
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kCurrentDir] = QUrl("file:///test");
    params[MenuParamKey::kIsEmptyArea] = false;
    
    QList<QUrl> selectedUrls;
    selectedUrls << QUrl("file:///test1") << QUrl("file:///test2");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectedUrls);
    
    scene->initialize(params);
    
    QAction action("Test Action");
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Triggered_WithCustomActionId_HandlesCorrectly)
{
    // Test triggered with custom action ID
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    
    scene->initialize(params);
    
    QAction action("Test Action");
    action.setProperty(ActionPropertyKey::kActionID, "custom-action-id");
    
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Create_WithValidParent_CreatesMenu)
{
    // Test create with valid parent menu
    // Use stub to avoid Q_ASSERT failure due to null view
    QMenu parentMenu;
    
    // Stub the qobject_cast to return a valid pointer
    using QObjCastFunc = FileView* (*)(QObject*);
    stub.set_lamda((QObjCastFunc)qobject_cast<FileView*>, [](QObject *obj) -> FileView* {
        Q_UNUSED(obj);
        // Return a mock FileView pointer to avoid Q_ASSERT
        static int mockView = 0;
        return reinterpret_cast<FileView*>(&mockView);
    });
    
    bool result = scene->create(&parentMenu);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, Create_WithInitializedParams_CreatesMenu)
{
    // Test create with initialized parameters
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kIsEmptyArea] = true;
    
    scene->initialize(params);
    
    // Use stub to avoid Q_ASSERT failure due to null view
    QMenu parentMenu;
    
    // Stub the qobject_cast to return a valid pointer
    using QObjCastFunc = FileView* (*)(QObject*);
    stub.set_lamda((QObjCastFunc)qobject_cast<FileView*>, [](QObject *obj) -> FileView* {
        Q_UNUSED(obj);
        // Return a mock FileView pointer to avoid Q_ASSERT
        static int mockView = 0;
        return reinterpret_cast<FileView*>(&mockView);
    });
    
    bool result = scene->create(&parentMenu);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceMenuSceneTriggerTest, UpdateState_WithCreatedMenu_UpdatesState)
{
    // Test updateState with created menu
    // Use stub to avoid Q_ASSERT failure due to null view
    QMenu parentMenu;
    
    // Stub the qobject_cast to return a valid pointer
    using QObjCastFunc = FileView* (*)(QObject*);
    stub.set_lamda((QObjCastFunc)qobject_cast<FileView*>, [](QObject *obj) -> FileView* {
        Q_UNUSED(obj);
        // Return a mock FileView pointer to avoid Q_ASSERT
        static int mockView = 0;
        return reinterpret_cast<FileView*>(&mockView);
    });
    
    scene->create(&parentMenu);
    
    // This should not crash
    EXPECT_NO_THROW(scene->updateState(&parentMenu));
}

TEST_F(WorkspaceMenuSceneTriggerTest, Scene_WithCreatedMenuAction_ReturnsCorrectScene)
{
    // Test scene with created menu action
    // Use stub to avoid Q_ASSERT failure due to null view
    QMenu parentMenu;
    
    // Stub the qobject_cast to return a valid pointer
    using QObjCastFunc = FileView* (*)(QObject*);
    stub.set_lamda((QObjCastFunc)qobject_cast<FileView*>, [](QObject *obj) -> FileView* {
        Q_UNUSED(obj);
        // Return a mock FileView pointer to avoid Q_ASSERT
        static int mockView = 0;
        return reinterpret_cast<FileView*>(&mockView);
    });
    
    scene->create(&parentMenu);
    
    QList<QAction*> actions = parentMenu.actions();
    if (!actions.isEmpty()) {
        AbstractMenuScene *result = scene->scene(actions.first());
        // Should return this scene for actions created by it
        EXPECT_EQ(result, scene);
    }
}

TEST_F(WorkspaceMenuSceneTriggerTest, Scene_WithExternalAction_ReturnsNull)
{
    // Test scene with external action
    QAction externalAction("External Action");
    
    AbstractMenuScene *result = scene->scene(&externalAction);
    
    EXPECT_EQ(result, nullptr);
}