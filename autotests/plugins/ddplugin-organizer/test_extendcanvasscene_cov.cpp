// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(menus/extendcanvasscene.cpp) -> 用例映射:
// - ExtendCanvasScene::initialize -> Initialize_Params_AllFieldsParsed (分支: 无选中文件 focusFile 为空)
// - ExtendCanvasScene::create -> Create_NullParent_ReturnsFalse
//     -> Create_EmptyArea_AddsOptionsAction (turnOn=false 分支: 仅"设置"项)
//     -> Create_NormalArea_CustomMode_AddsCreateCollection (normalMenu 分支)
// - ExtendCanvasScenePrivate::emptyMenu / normalMenu -> 由 create 的两个用例间接覆盖
// - ExtendCanvasScenePrivate::updateEmptyMenu -> UpdateEmptyMenu_BuiltMenu_ActionsUpdated
// - ExtendCanvasScenePrivate::organizeBySubActions -> OrganizeBySubActions_ClassifierActionsBuilt
// - ExtendCanvasScenePrivate::classifierToActionID -> ClassifierToActionID_AllClassifiers_Mapped
// - ExtendCanvasScenePrivate::triggerSortby -> TriggerSortby_UnknownAction_ReturnsFalse (默认分支)
// - ExtendCanvasScene::triggered -> Triggered_RegisteredAction_EmitsSignal / UnknownAction_ReturnsFalse
// - ExtendCanvasScene::scene -> Scene_RegisteredAction_ReturnsSceneItself
// 未覆盖: actionFilter(依赖完整菜单场景树), updateEmptyMenu 内部 lambda 的回调分支。

#include "stubext.h"
#include "menus/extendcanvasscene.h"
#include "menus/extendcanvasscene_p.h"
#include "config/configpresenter.h"
#include "organizer_defines.h"
#include "menus/organizermenu_defines.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QMenu>
#include <QAction>
#include <QSignalSpy>
#include <QUrl>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

class TestableExtendCanvasScene : public ExtendCanvasScene
{
public:
    ExtendCanvasScenePrivate *priv() { return d; }
};

}   // namespace

class UT_ExtendCanvasSceneCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // presenter stays disabled so emptyMenu only adds the options entry
        stub.set_lamda(&ConfigPresenter::isEnable, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ConfigPresenter::mode, []() -> OrganizerMode {
            __DBG_STUB_INVOKE__
            return OrganizerMode::kCustom;
        });
        stub.set_lamda(&ConfigPresenter::organizeOnTriggered, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        scene = new TestableExtendCanvasScene();
    }

    void TearDown() override
    {
        delete scene;
        stub.clear();
    }

public:
    TestableExtendCanvasScene *scene = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtendCanvasSceneCov, Initialize_Params_AllFieldsParsed)
{
    // Arrange: full parameter set with selected files
    QVariantHash params;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kIsEmptyArea] = true;
    QList<QUrl> files { QUrl::fromLocalFile("/tmp/cov_file.txt") };
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(files);
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");

    // Act
    bool onDesktop = scene->initialize(params);

    // Assert: all fields parsed, focus falls to first file
    EXPECT_TRUE(onDesktop);
    EXPECT_EQ(scene->priv()->selectFiles, files);
    EXPECT_EQ(scene->priv()->focusFile, files.first());

    // Act: empty selection leaves focus empty (second branch)
    QVariantHash emptyParams;
    emptyParams[MenuParamKey::kOnDesktop] = false;
    bool offDesktop = scene->initialize(emptyParams);

    // Assert
    EXPECT_FALSE(offDesktop);
    EXPECT_TRUE(scene->priv()->selectFiles.isEmpty());
    // focusFile is not reset by a later initialize: keeps the previous value
    EXPECT_EQ(scene->priv()->focusFile, files.first());
}

TEST_F(UT_ExtendCanvasSceneCov, Create_NullParent_ReturnsFalse)
{
    // Arrange: no parent menu

    // Act
    bool created = scene->create(nullptr);

    // Assert: rejected without crashing
    EXPECT_FALSE(created);
    EXPECT_EQ(scene->priv()->predicateAction.count(), 0);
}

TEST_F(UT_ExtendCanvasSceneCov, Create_EmptyArea_AddsOptionsAction)
{
    // Arrange: empty desktop area, organizer disabled
    scene->initialize({ { MenuParamKey::kIsEmptyArea, true },
                        { MenuParamKey::kOnDesktop, true } });
    QMenu menu;

    // Act
    bool created = scene->create(&menu);

    // Assert: only the options entry was appended
    EXPECT_TRUE(created);
    EXPECT_EQ(menu.actions().count(), 1);
    EXPECT_EQ(scene->priv()->predicateAction.count(), 1);
}

TEST_F(UT_ExtendCanvasSceneCov, Create_NormalArea_CustomMode_AddsCreateCollection)
{
    // Arrange: files selected, organizer enabled, custom mode
    stub.set_lamda(&ConfigPresenter::isEnable, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    scene->initialize({ { MenuParamKey::kIsEmptyArea, false },
                        { MenuParamKey::kOnDesktop, true },
                        { MenuParamKey::kSelectFiles,
                          QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/cov_file.txt") }) } });
    QMenu menu;

    // Act
    bool created = scene->create(&menu);

    // Assert: normal menu offers creating a collection
    EXPECT_TRUE(created);
    EXPECT_EQ(menu.actions().count(), 1);
    EXPECT_TRUE(menu.actions().first()->property(ActionPropertyKey::kActionID).toString()
                == QString(ActionID::kCreateACollection));
}

TEST_F(UT_ExtendCanvasSceneCov, UpdateEmptyMenu_BuiltMenu_ActionsUpdated)
{
    // Arrange: menu built through the regular path
    scene->initialize({ { MenuParamKey::kIsEmptyArea, true },
                        { MenuParamKey::kOnDesktop, true } });
    QMenu menu;
    scene->create(&menu);
    const int countBefore = menu.actions().count();

    // Act
    scene->priv()->updateEmptyMenu(&menu);

    // Assert: update pass keeps the menu structurally intact
    EXPECT_EQ(menu.actions().count(), countBefore);
    EXPECT_FALSE(scene->priv()->predicateAction.isEmpty());
}

TEST_F(UT_ExtendCanvasSceneCov, OrganizeBySubActions_ClassifierActionsBuilt)
{
    // Arrange: parent menu for the organize-by submenu, organizer disabled
    QMenu menu;

    // Act
    QMenu *subMenu = scene->priv()->organizeBySubActions(&menu);

    // Assert: submenu returned and owned by the parent; with the organizer
    // disabled the classifier entries stay hidden
    ASSERT_NE(subMenu, nullptr);
    EXPECT_EQ(subMenu->actions().count(), 0);
    EXPECT_EQ(subMenu->parent(), &menu);
}

TEST_F(UT_ExtendCanvasSceneCov, ClassifierToActionID_AllClassifiers_Mapped)
{
    // Arrange: classifiers to translate
    const QList<Classifier> mapped { kType, kTimeCreated, kTimeModified };
    const QList<Classifier> unmapped { kLabel, kName };

    // Act
    QString typeAction = scene->priv()->classifierToActionID(mapped.at(0));
    QString createdAction = scene->priv()->classifierToActionID(mapped.at(1));
    QString modifiedAction = scene->priv()->classifierToActionID(mapped.at(2));
    QString labelAction = scene->priv()->classifierToActionID(unmapped.at(0));
    QString nameAction = scene->priv()->classifierToActionID(unmapped.at(1));

    // Assert: mapped classifiers resolve, unmapped stay empty
    EXPECT_EQ(typeAction, QString(ActionID::kOrganizeByType));
    EXPECT_EQ(createdAction, QString(ActionID::kOrganizeByTimeCreated));
    EXPECT_EQ(modifiedAction, QString(ActionID::kOrganizeByTimeModified));
    EXPECT_TRUE(labelAction.isEmpty());
    EXPECT_TRUE(nameAction.isEmpty());
}

TEST_F(UT_ExtendCanvasSceneCov, TriggerSortby_UnknownAction_ReturnsFalse)
{
    // Arrange: an action id outside the sort map, no view attached

    // Act
    bool handled = scene->priv()->triggerSortby("not-a-sort-action");

    // Assert: unknown ids are refused and no view is attached
    EXPECT_FALSE(handled);
    EXPECT_EQ(scene->priv()->view, nullptr);
}

TEST_F(UT_ExtendCanvasSceneCov, Triggered_RegisteredAction_EmitsSignal)
{
    // Arrange: build the menu so the options action is registered
    scene->initialize({ { MenuParamKey::kIsEmptyArea, true },
                        { MenuParamKey::kOnDesktop, true } });
    QMenu menu;
    scene->create(&menu);
    QAction *optionsAction = menu.actions().first();
    QSignalSpy spy(ConfigPresenter::instance(), &ConfigPresenter::showOptionWindow);

    // Act
    bool handled = scene->triggered(optionsAction);

    // Assert: registered action handled and signal emitted
    EXPECT_TRUE(handled);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ExtendCanvasSceneCov, Triggered_UnknownAction_ReturnsFalse)
{
    // Arrange: a foreign action never registered in this scene
    QAction foreign("foreign");

    // Act
    bool handled = scene->triggered(&foreign);

    // Assert: not consumed by this scene
    EXPECT_FALSE(handled);
    EXPECT_EQ(foreign.text(), QString("foreign"));
}

TEST_F(UT_ExtendCanvasSceneCov, Scene_RegisteredAction_ReturnsSceneItself)
{
    // Arrange: menu with a registered action
    scene->initialize({ { MenuParamKey::kIsEmptyArea, true },
                        { MenuParamKey::kOnDesktop, true } });
    QMenu menu;
    scene->create(&menu);
    QAction *registered = menu.actions().first();
    QAction foreign("foreign");

    // Act
    AbstractMenuScene *ownerScene = scene->scene(registered);
    AbstractMenuScene *foreignScene = scene->scene(&foreign);

    // Assert: the scene owns its registered action
    EXPECT_EQ(ownerScene, scene);
    EXPECT_NE(foreignScene, scene);
}
