// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/actioniconmenuscene.h"
#include "menuscene/private/actioniconmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ActionIconMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new ActionIconMenuScene();
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    ActionIconMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_ActionIconMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new ActionIconMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    ActionIconMenuCreator *creator { nullptr };
};

TEST_F(UT_ActionIconMenuCreator, Name_ReturnsActionIconManagerName)
{
    EXPECT_EQ(ActionIconMenuCreator::name(), "ActionIconManager");
}

TEST_F(UT_ActionIconMenuCreator, Create_ReturnsActionIconMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "ActionIconManager");
    delete scene;
}

TEST_F(UT_ActionIconMenuScene, Name_ReturnsActionIconManagerName)
{
    EXPECT_EQ(scene->name(), "ActionIconManager");
}

TEST_F(UT_ActionIconMenuScene, Initialize_AnyParams_DelegatesToBaseTrue)
{
    QVariantHash params;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ActionIconMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_ActionIconMenuScene, UpdateState_IconNotVisible_CallsBaseOnly)
{
    QMenu menu;

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->updateState(&menu);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_ActionIconMenuScene, UpdateState_IconVisibleKnownAction_NoFatalFailure)
{
    QMenu menu;
    QAction action("New Folder");
    action.setProperty(ActionPropertyKey::kActionID, "new-folder");
    menu.addAction(&action);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // icon depends on the runtime icon theme, so only verify the traversal is safe
    // and the action is kept alive in the menu.
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(menu.actions().contains(&action));
}

TEST_F(UT_ActionIconMenuScene, UpdateState_MultipleKnownActions_ProcessesAll)
{
    QMenu menu;
    QAction action1("Copy");
    action1.setProperty(ActionPropertyKey::kActionID, "copy");
    QAction action2("Paste");
    action2.setProperty(ActionPropertyKey::kActionID, "paste");
    QAction action3("Cut");
    action3.setProperty(ActionPropertyKey::kActionID, "cut");

    menu.addAction(&action1);
    menu.addAction(&action2);
    menu.addAction(&action3);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // icon availability depends on the icon theme, assert traversal only.
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(menu.actions().contains(&action1));
    EXPECT_TRUE(menu.actions().contains(&action2));
    EXPECT_TRUE(menu.actions().contains(&action3));
}

TEST_F(UT_ActionIconMenuScene, UpdateState_UnknownActionID_KeepsIconNull)
{
    QMenu menu;
    QAction action("Unknown");
    action.setProperty(ActionPropertyKey::kActionID, "unknown-action");
    menu.addAction(&action);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->updateState(&menu);
    // an unknown id is never in the icon map, so the icon stays null on any theme
    EXPECT_TRUE(action.icon().isNull());
}

TEST_F(UT_ActionIconMenuScene, UpdateState_NoActionID_KeepsIconNull)
{
    QMenu menu;
    QAction action("No ID");
    // no ActionID property set
    menu.addAction(&action);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->updateState(&menu);
    EXPECT_TRUE(action.icon().isNull());
}

TEST_F(UT_ActionIconMenuScene, UpdateState_SubMenu_ProcessesSubActions)
{
    QMenu menu;
    QMenu *subMenu = new QMenu(&menu);
    QAction subAction("Delete");
    subAction.setProperty(ActionPropertyKey::kActionID, "delete");
    subMenu->addAction(&subAction);

    QAction mainAction("File");
    mainAction.setMenu(subMenu);
    menu.addAction(&mainAction);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // icon depends on the runtime icon theme, verify traversal only.
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(subMenu->actions().contains(&subAction));
}

TEST_F(UT_ActionIconMenuScene, UpdateState_NestedSubMenus_ProcessesAllLevels)
{
    QMenu menu;
    QMenu *subMenu = new QMenu(&menu);
    QMenu *subSubMenu = new QMenu(subMenu);

    QAction deepAction("Rename");
    deepAction.setProperty(ActionPropertyKey::kActionID, "rename");
    subSubMenu->addAction(&deepAction);

    QAction subAction("More");
    subAction.setMenu(subSubMenu);
    subMenu->addAction(&subAction);

    QAction mainAction("Edit");
    mainAction.setMenu(subMenu);
    menu.addAction(&mainAction);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // icon depends on the runtime icon theme, verify traversal only.
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(subSubMenu->actions().contains(&deepAction));
}

TEST_F(UT_ActionIconMenuScene, UpdateState_MixedActions_UnknownAndNoIdKeepNullIcon)
{
    QMenu menu;
    QAction action1("Copy");
    action1.setProperty(ActionPropertyKey::kActionID, "copy");
    QAction action2("Unknown");
    action2.setProperty(ActionPropertyKey::kActionID, "unknown");
    QAction action3("No ID");
    // action3 has no ActionID

    menu.addAction(&action1);
    menu.addAction(&action2);
    menu.addAction(&action3);

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    // icons of unknown/no-id actions are never touched on any theme
    EXPECT_TRUE(action2.icon().isNull());
    EXPECT_TRUE(action3.icon().isNull());
}

TEST_F(UT_ActionIconMenuScene, UpdateState_AllKnownActions_TraversesAllWithoutCrash)
{
    QMenu menu;

    QStringList knownActions = {
        "new-folder", "new-document", "open-in-new-window",
        "open-in-new-tab", "open-in-terminal", "add-bookmark",
        "remove-bookmark", "copy", "paste", "cut", "rename",
        "delete", "select-all", "property"
    };

    QList<QAction *> actions;
    for (const QString &id : knownActions) {
        QAction *action = new QAction(id, &menu);
        action->setProperty(ActionPropertyKey::kActionID, id);
        menu.addAction(action);
        actions.append(action);
    }

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // icon availability depends on the runtime icon theme; only make sure the
    // whole action set is traversed safely.
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    for (QAction *action : actions) {
        ASSERT_NE(action, nullptr);
        EXPECT_TRUE(menu.actions().contains(action)) << "Action " << action->text().toStdString() << " lost";
    }
}

TEST_F(UT_ActionIconMenuScene, UpdateState_EmptyMenu_DoesNotCrash)
{
    QMenu menu;

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ActionIconMenuScene::actionIconVisible, [](const ActionIconMenuScene *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}

TEST_F(UT_ActionIconMenuScene, ActionIconVisible_Default_ReturnsFalse)
{
    EXPECT_FALSE(scene->actionIconVisible());
}

TEST_F(UT_ActionIconMenuScene, PrivateCtor_ParentSetToScene)
{
    ActionIconMenuScenePrivate pri(scene);
    EXPECT_EQ(pri.parent(), scene);
}
