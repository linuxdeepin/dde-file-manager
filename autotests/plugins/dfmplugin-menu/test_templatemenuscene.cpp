// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "templatemenuscene/templatemenuscene.h"
#include "templatemenuscene/templatemenu.h"
#include "templatemenuscene/private/templatemenuscene_p.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QAction>
#include <QMenu>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_TemplateMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new TemplateMenu;
        scene = new TemplateMenuScene(menu);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete menu;
        menu = nullptr;
        stub.clear();
    }

protected:
    TemplateMenuScene *scene { nullptr };
    TemplateMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

class UT_TemplateMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new TemplateMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    TemplateMenuCreator *creator { nullptr };
};

TEST_F(UT_TemplateMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(TemplateMenuCreator::name(), "TemplateMenu");
}

TEST_F(UT_TemplateMenuCreator, Create_ReturnsTemplateMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "TemplateMenu");
    delete scene;
}

TEST_F(UT_TemplateMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "TemplateMenu");
}

TEST_F(UT_TemplateMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_TemplateMenuScene, Initialize_NotEmptyArea_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // 非空白区域时不应继续初始化（也不会调用基类）
    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_TemplateMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_TemplateMenuScene, Scene_PredicateActionMatch_ReturnsSelf)
{
    QAction *act = new QAction("template-action");
    scene->d->predicateAction.insert("some-id", act);

    EXPECT_EQ(scene->scene(act), scene);

    delete act;
}

TEST_F(UT_TemplateMenuScene, Scene_ForeignAction_DelegatesToBase)
{
    QAction *act = new QAction("template-action");
    QAction *foreign = new QAction("foreign-action");
    scene->d->predicateAction.insert("some-id", act);

    // 未命中 predicateAction 且无子场景，基类返回 nullptr
    EXPECT_EQ(scene->scene(foreign), nullptr);

    delete act;
    delete foreign;
}

TEST_F(UT_TemplateMenuScene, Create_NewDocMenuFound_AddsTemplateActionsToSubMenu)
{
    QMenu parentMenu;
    QMenu *subMenu = new QMenu(&parentMenu);
    QAction *newDocAction = parentMenu.addAction("New Document");
    newDocAction->setProperty(ActionPropertyKey::kActionID, ActionID::kNewDoc);
    newDocAction->setMenu(subMenu);

    QAction *templateAction = new QAction("Blank Doc");
    templateAction->setData(QVariant::fromValue(QString("/tmp/ut-template-file.doc")));
    scene->d->templateActions.append(templateAction);

    EXPECT_TRUE(scene->create(&parentMenu));

    // 模板 action 被加入 new-doc 子菜单
    EXPECT_EQ(subMenu->actions().count(templateAction), 1);
    const QString assignedId = templateAction->property(ActionPropertyKey::kActionID).toString();
    EXPECT_FALSE(assignedId.isEmpty());
    EXPECT_TRUE(assignedId.startsWith("TemplateMenu:"));
    // predicate 注册表已登记
    EXPECT_EQ(scene->d->predicateAction.value(assignedId), templateAction);
    EXPECT_EQ(scene->d->predicateName.value(assignedId), QString("Blank Doc"));
}

TEST_F(UT_TemplateMenuScene, Create_NewDocActionWithoutSubMenu_NoActionAdded)
{
    QMenu parentMenu;
    QAction *newDocAction = parentMenu.addAction("New Document");
    newDocAction->setProperty(ActionPropertyKey::kActionID, ActionID::kNewDoc);
    // 不设置 subMenu

    QAction *templateAction = new QAction("Blank Doc");
    scene->d->templateActions.append(templateAction);

    EXPECT_TRUE(scene->create(&parentMenu));
    EXPECT_EQ(parentMenu.actions().count(templateAction), 0);
}

TEST_F(UT_TemplateMenuScene, Create_NoNewDocAction_DelegatesToBase)
{
    QMenu parentMenu;
    parentMenu.addAction("plain-action");

    QAction *templateAction = new QAction("Blank Doc");
    scene->d->templateActions.append(templateAction);

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, create),
                   [&baseCalled](AbstractMenuScene *, QMenu *) -> bool {
                       __DBG_STUB_INVOKE__
                       baseCalled = true;
                       return true;
                   });

    EXPECT_TRUE(scene->create(&parentMenu));
    EXPECT_TRUE(baseCalled);
    EXPECT_EQ(parentMenu.actions().count(templateAction), 0);
}

TEST_F(UT_TemplateMenuScene, UpdateState_ForwardsToBase)
{
    QMenu parentMenu;
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState),
                   [&baseCalled](AbstractMenuScene *, QMenu *) {
                       __DBG_STUB_INVOKE__
                       baseCalled = true;
                   });

    scene->updateState(&parentMenu);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_TemplateMenuScene, UpdateState_NullMenu_NotForwarded)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState),
                   [&baseCalled](AbstractMenuScene *, QMenu *) {
                       __DBG_STUB_INVOKE__
                       baseCalled = true;
                   });

    scene->updateState(nullptr);
    EXPECT_FALSE(baseCalled);
}

TEST_F(UT_TemplateMenuScene, Triggered_OwnAction_PublishesEventAndReturnsTrue)
{
    QAction *act = new QAction("Blank Doc");
    act->setData(QVariant::fromValue(QString("/tmp/ut-template-file.doc")));
    const QString id = "TemplateMenu:ut-test-id";
    act->setProperty(ActionPropertyKey::kActionID, id);
    scene->d->predicateAction.insert(id, act);
    scene->d->windowId = 1;
    scene->d->currentDir = QUrl::fromLocalFile("/tmp");

    EXPECT_TRUE(scene->triggered(act));

    delete act;
}

TEST_F(UT_TemplateMenuScene, Triggered_ForeignAction_DelegatesToBase)
{
    QAction *foreign = new QAction("foreign");

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered),
                   [&baseCalled](AbstractMenuScene *, QAction *) -> bool {
                       __DBG_STUB_INVOKE__
                       baseCalled = true;
                       return false;
                   });

    EXPECT_FALSE(scene->triggered(foreign));
    EXPECT_TRUE(baseCalled);

    delete foreign;
}
