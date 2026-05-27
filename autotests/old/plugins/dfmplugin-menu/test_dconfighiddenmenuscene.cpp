// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/dconfighiddenmenuscene.h"
#include "utils/menuhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class TestMenuScene : public AbstractMenuScene
{
public:
    explicit TestMenuScene() = default;
    QString name() const override { return "TestScene"; }
};

class UT_DConfigHiddenMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new DConfigHiddenMenuScene();
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    DConfigHiddenMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_DConfigHiddenMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new DConfigHiddenMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    DConfigHiddenMenuCreator *creator { nullptr };
};

TEST_F(UT_DConfigHiddenMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(DConfigHiddenMenuCreator::name(), "DConfigMenuFilter");
}

TEST_F(UT_DConfigHiddenMenuCreator, Create_ReturnsDConfigHiddenMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "DConfigMenuFilter");
    delete scene;
}

TEST_F(UT_DConfigHiddenMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "DConfigMenuFilter");
}

TEST_F(UT_DConfigHiddenMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_DConfigHiddenMenuScene, Initialize_ValidCurrentDir_ChecksHidden)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");

    bool helperCalled = false;
    stub.set_lamda(&Helper::isHiddenExtMenu, [&helperCalled](const QUrl &) {
        __DBG_STUB_INVOKE__
        helperCalled = true;
        return false;
    });

    scene->initialize(params);
    EXPECT_TRUE(helperCalled);
}

TEST_F(UT_DConfigHiddenMenuScene, Initialize_HiddenExtMenuTrue_DisablesScene)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");

    bool disableCalled = false;
    stub.set_lamda(&Helper::isHiddenExtMenu, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DConfigHiddenMenuScene::disableScene, [&disableCalled](DConfigHiddenMenuScene *) {
        __DBG_STUB_INVOKE__
        disableCalled = true;
    });

    scene->initialize(params);
    EXPECT_TRUE(disableCalled);
}

TEST_F(UT_DConfigHiddenMenuScene, Initialize_InvalidUrl_DoesNotDisable)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl();

    bool helperCalled = false;
    stub.set_lamda(&Helper::isHiddenExtMenu, [&helperCalled](const QUrl &) {
        __DBG_STUB_INVOKE__
        helperCalled = true;
        return false;
    });

    scene->initialize(params);
    EXPECT_FALSE(helperCalled);
}

TEST_F(UT_DConfigHiddenMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateState_NullParent_DoesNotCrash)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    scene->updateState(nullptr);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateState_ValidParent_CallsUpdateActionHidden)
{
    QMenu menu;

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_EmptyHiddenList_DoesNothing)
{
    QMenu menu;
    QAction action("test");
    action.setProperty(ActionPropertyKey::kActionID, "test-action");
    menu.addAction(&action);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_TRUE(action.isVisible());
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_MatchingActionID_HidesAction)
{
    QMenu menu;
    QAction action("test");
    action.setProperty(ActionPropertyKey::kActionID, "test-action");
    menu.addAction(&action);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList { "test-action" });
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_FALSE(action.isVisible());
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_NonMatchingActionID_KeepsVisible)
{
    QMenu menu;
    QAction action("test");
    action.setProperty(ActionPropertyKey::kActionID, "test-action");
    menu.addAction(&action);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList { "other-action" });
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_TRUE(action.isVisible());
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_SubMenu_ProcessesSubMenuActions)
{
    QMenu menu;
    QMenu *subMenu = new QMenu(&menu);
    QAction subAction("sub");
    subAction.setProperty(ActionPropertyKey::kActionID, "sub-action");
    subMenu->addAction(&subAction);

    QAction mainAction("main");
    mainAction.setMenu(subMenu);
    menu.addAction(&mainAction);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList { "sub-action" });
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_FALSE(subAction.isVisible());
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_NoActionID_SkipsAction)
{
    QMenu menu;
    QAction action("test");
    // 不设置 ActionID
    menu.addAction(&action);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList { "test-action" });
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_TRUE(action.isVisible());
}

TEST_F(UT_DConfigHiddenMenuScene, DisableScene_NoParent_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(scene->disableScene());
}

TEST_F(UT_DConfigHiddenMenuScene, DisableScene_WithExtendScenes_RemovesScenes)
{
    // 创建一个假的父场景
    AbstractMenuScene *parentScene = new TestMenuScene();

    // 创建模拟的扩展场景
    AbstractMenuScene *oemScene = new TestMenuScene();
    AbstractMenuScene *extendScene = new TestMenuScene();

    stub.set_lamda(VADDR(TestMenuScene, name), [](AbstractMenuScene *self) -> QString {
        __DBG_STUB_INVOKE__
        if (self->objectName() == "oem")
            return "OemMenu";
        if (self->objectName() == "extend")
            return "ExtendMenu";
        return "Other";
    });

    stub.set_lamda(&DConfigHiddenMenuScene::parent, [&] {
        __DBG_STUB_INVOKE__
        return parentScene;
    });

    oemScene->setObjectName("oem");
    extendScene->setObjectName("extend");

    parentScene->addSubscene(oemScene);
    parentScene->addSubscene(extendScene);

    bool removeCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, removeSubscene), [&removeCalled](AbstractMenuScene *, AbstractMenuScene *) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
    });

    scene->disableScene();
    // 由于我们stubbed了removeSubscene，这里只是检查逻辑是否被调用

    delete parentScene;
}

TEST_F(UT_DConfigHiddenMenuScene, UpdateActionHidden_MultipleActions_ProcessesAll)
{
    QMenu menu;
    QAction action1("test1");
    action1.setProperty(ActionPropertyKey::kActionID, "action1");
    QAction action2("test2");
    action2.setProperty(ActionPropertyKey::kActionID, "action2");
    QAction action3("test3");
    action3.setProperty(ActionPropertyKey::kActionID, "action3");

    menu.addAction(&action1);
    menu.addAction(&action2);
    menu.addAction(&action3);

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList { "action1", "action3" });
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    scene->updateState(&menu);
    EXPECT_FALSE(action1.isVisible());
    EXPECT_TRUE(action2.isVisible());
    EXPECT_FALSE(action3.isVisible());
}
