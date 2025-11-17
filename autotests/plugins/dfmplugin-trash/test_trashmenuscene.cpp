// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <QMenu>

#include "menus/trashmenuscene.h"
#include "utils/trashhelper.h"
#include "dfmplugin_trash_global.h"

#include <stubext.h>

#include <dfm-base/dfm_menu_defines.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestTrashMenuScene, MenuCreatorName)
{
    QString name = TrashMenuCreator::name();
    EXPECT_EQ(name, "TrashMenu");
}

TEST_F(TestTrashMenuScene, MenuCreatorCreate)
{
    TrashMenuCreator creator;
    AbstractMenuScene *scene = creator.create();
    
    EXPECT_NE(scene, nullptr);
    TrashMenuScene *trashScene = dynamic_cast<TrashMenuScene*>(scene);
    EXPECT_NE(trashScene, nullptr);
    
    delete scene;
}

TEST_F(TestTrashMenuScene, Constructor)
{
    TrashMenuScene scene;
    // Just test that constructor works without crash
    EXPECT_TRUE(true);
}

TEST_F(TestTrashMenuScene, Name)
{
    TrashMenuScene scene;
    QString name = scene.name();
    EXPECT_EQ(name, "TrashMenu");
}

TEST_F(TestTrashMenuScene, Initialize)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    bool result = scene.initialize(params);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashMenuScene, Create_EmptyArea)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    QMenu *parent = new QMenu();
    bool result = scene.create(parent);
    EXPECT_TRUE(result);
    
    // Check if actions were added to the menu
    EXPECT_GT(parent->actions().size(), 0);
    
    delete parent;
}

TEST_F(TestTrashMenuScene, Create_SelectedFiles)
{
    TrashMenuScene scene;
    
    // Initialize first with non-empty area
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, false);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    QMenu *parent = new QMenu();
    bool result = scene.create(parent);
    EXPECT_TRUE(result);
    
    // Check if actions were added to the menu
    EXPECT_GT(parent->actions().size(), 0);
    
    delete parent;
}

TEST_F(TestTrashMenuScene, UpdateState)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    QMenu *parent = new QMenu();
    scene.create(parent);
    
    scene.updateState(parent);
    // If we reach here without crash, updateState worked
    EXPECT_TRUE(true);
    
    delete parent;
}

TEST_F(TestTrashMenuScene, Triggered_Restore)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, false);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    // Create a test action for restore
    QAction *action = new QAction();
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, TrashActionId::kRestore);
    
    bool restoreHandleCalled = false;
    stub.set_lamda(&TrashHelper::restoreFromTrashHandle,
        [&restoreHandleCalled](quint64 windowId, const QList<QUrl> urls, 
                              const AbstractJobHandler::JobFlags flags) {
            Q_UNUSED(windowId)
            Q_UNUSED(urls)
            Q_UNUSED(flags)
            restoreHandleCalled = true;
            return JobHandlePointer();
        });
    
    bool result = scene.triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(restoreHandleCalled);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Triggered_RestoreAll)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    // Create a test action for restore all
    QAction *action = new QAction();
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, TrashActionId::kRestoreAll);
    
    bool restoreHandleCalled = false;
    stub.set_lamda(&TrashHelper::restoreFromTrashHandle,
        [&restoreHandleCalled](quint64 windowId, const QList<QUrl> urls, 
                              const AbstractJobHandler::JobFlags flags) {
            Q_UNUSED(windowId)
            Q_UNUSED(urls)
            Q_UNUSED(flags)
            restoreHandleCalled = true;
            return JobHandlePointer();
        });
    
    bool result = scene.triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(restoreHandleCalled);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Triggered_EmptyTrash)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    // Create a test action for empty trash
    QAction *action = new QAction();
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, TrashActionId::kEmptyTrash);
    
    bool emptyTrashCalled = false;
    stub.set_lamda(&TrashHelper::emptyTrash,
        [&emptyTrashCalled](quint64 windowId) {
            Q_UNUSED(windowId)
            emptyTrashCalled = true;
        });
    
    bool result = scene.triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(emptyTrashCalled);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Triggered_SortBySourcePath)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    // Create a test action for sort by source path
    QAction *action = new QAction();
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, TrashActionId::kSourcePath);
    
    // Test that triggered returns true for valid sort action
    bool result = scene.triggered(action);
    EXPECT_TRUE(result);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Triggered_SortByTimeDeleted)
{
    TrashMenuScene scene;
    
    // Initialize first
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kCurrentDir, QUrl("trash:///"));
    params.insert(dfmbase::MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>{QUrl("trash:///test.txt")}));
    params.insert(dfmbase::MenuParamKey::kOnDesktop, false);
    params.insert(dfmbase::MenuParamKey::kIsEmptyArea, true);
    params.insert(dfmbase::MenuParamKey::kWindowId, 12345ULL);
    params.insert(dfmbase::MenuParamKey::kIndexFlags, QVariant::fromValue(Qt::ItemFlags()));
    
    scene.initialize(params);
    
    // Create a test action for sort by time deleted
    QAction *action = new QAction();
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, TrashActionId::kTimeDeleted);
    
    // Test that triggered returns true for valid sort action
    bool result = scene.triggered(action);
    EXPECT_TRUE(result);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Scene)
{
    TrashMenuScene scene;
    
    // Create a test action
    QAction *action = new QAction();
    QString actionId = "test_action";
    action->setProperty(dfmbase::ActionPropertyKey::kActionID, actionId);
    
    AbstractMenuScene *resultScene = scene.scene(action);
    // The result might be null if the action is not in predicateAction map
    // But we just test that the method runs without crash
    EXPECT_TRUE(true);
    
    delete action;
}

TEST_F(TestTrashMenuScene, Destructor)
{
    TrashMenuScene *scene = new TrashMenuScene();
    delete scene;
    // Test that destructor works without crash
    EXPECT_TRUE(true);
}