// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menus/extendcanvasscene_p.h"
#include "menus/organizermenu_defines.h"
#include "config/configpresenter.h"
#include "view/collectionview.h"
#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "utils/renamedialog.h"
#include <dfm-base/dfm_menu_defines.h>
#include "core/ddplugin-canvas/menu/canvasmenu_defines.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <DDialog>
#include <QAction>
#include <QMenu>
DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_organizer;

DWIDGET_USE_NAMESPACE
TEST(ExtendCanvasScene, initialize_empty)
{
    QUrl dir("file://desktop");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = dir;
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kWindowId] = 1;
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = 2;

    {
        ExtendCanvasScene scene;
        EXPECT_FALSE(scene.initialize(params));
    }

    {
        stub_ext::StubExt stub;
        stub.set_lamda(&ConfigPresenter::isEnable, []() {
            return true;
        });

        ExtendCanvasScene scene;
        params[MenuParamKey::kOnDesktop] = true;
        EXPECT_TRUE(scene.initialize(params));
        EXPECT_EQ(scene.d->turnOn, true);
        EXPECT_EQ(scene.d->isEmptyArea, true);
        EXPECT_EQ(scene.d->onDesktop, true);
        EXPECT_EQ(scene.d->selectFiles.size(), 0);
        EXPECT_EQ(scene.d->onCollection, true);
        EXPECT_EQ(reinterpret_cast<qintptr>(scene.d->view), 2);
        EXPECT_EQ(scene.d->focusFile.isEmpty(), true);
        EXPECT_EQ(scene.d->currentDir, dir);
    }
}

TEST(ExtendCanvasScene, initialize_normal)
{
    QUrl dir("file://desktop");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = dir;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = 1;
    params[MenuParamKey::kIsEmptyArea] = false;
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = 2;
    QList<QUrl> selectUrls { QUrl("file://etc"), QUrl("file://usr") };
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);

    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::isEnable, []() {
        return false;
    });

    ExtendCanvasScene scene;
    EXPECT_TRUE(scene.initialize(params));
    EXPECT_EQ(scene.d->turnOn, false);
    EXPECT_EQ(scene.d->isEmptyArea, false);
    EXPECT_EQ(scene.d->onDesktop, true);
    EXPECT_EQ(scene.d->onCollection, true);
    EXPECT_EQ(reinterpret_cast<qintptr>(scene.d->view), 2);
    EXPECT_EQ(scene.d->currentDir, dir);

    ASSERT_EQ(scene.d->selectFiles, selectUrls);
    EXPECT_EQ(scene.d->focusFile, selectUrls.first());
}

TEST(ExtendCanvasScene, triggered_organizeoptions)
{
    ExtendCanvasScene scene;

    QAction tempAction;
    scene.d->predicateAction[ActionID::kOrganizeOptions] = &tempAction;
    tempAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeOptions));

    stub_ext::StubExt stub;
    bool show = false;
    stub.set_lamda(&ConfigPresenter::showOptionWindow, [&show]() {
        show = true;
    });

    EXPECT_TRUE(scene.triggered(&tempAction));
    EXPECT_TRUE(show);
}

TEST(ExtendCanvasScenePrivate, triggerSortby)
{
    ExtendCanvasScenePrivate obj(nullptr);

    EXPECT_FALSE(obj.triggerSortby("name"));
    EXPECT_FALSE(obj.triggerSortby("sort-by"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-name"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-time-modified"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-size"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-type"));
}

TEST(ExtendCanvasScenePrivate, Menu)
{
    QMenu parent;
    ExtendCanvasScene scene;
    scene.d->selectFiles.push_back(QUrl("temp_file"));
    scene.d->emptyMenu(&parent);
    scene.d->turnOn = true;
    CfgPresenter->curMode = OrganizerMode::kCustom;
    scene.d->normalMenu(&parent);
    {
        QAction action2;
        QAction action3;
        QAction action4;
        QAction action5;
        action2.setProperty(ActionPropertyKey::kActionID, ddplugin_canvas::ActionID::kSortBy);
        action3.setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSelectAll);
        action4.setProperty(ActionPropertyKey::kActionID, ddplugin_canvas::ActionID::kWallpaperSettings);
        action5.setProperty(ActionPropertyKey::kActionID, ddplugin_canvas::ActionID::kDisplaySettings);
        QList<QAction *> actions { &action2, &action3, &action4, &action5 };

        parent.addActions(actions);
        {
            CfgPresenter->curMode = OrganizerMode::kNormalized;
            scene.d->updateEmptyMenu(&parent);
            EXPECT_FALSE(action2.isVisible());
            EXPECT_FALSE(action3.isVisible());
            EXPECT_TRUE(action4.isVisible());
            EXPECT_TRUE(action5.isVisible());
        }
    }
    EXPECT_NO_FATAL_FAILURE(scene.d->updateNormalMenu(&parent));
}
TEST(ExtendCanvasScenePrivate, classifierToActionID)
{
    Classifier cf;
    ExtendCanvasScene scene;
    {
        cf = kType;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, ActionID::kOrganizeByType);
    }
    {
        cf = kTimeCreated;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, ActionID::kOrganizeByTimeCreated);
    }
    {
        cf = kTimeModified;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, ActionID::kOrganizeByTimeModified);
    }
    {
        cf = kLabel;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, "");
    }
    {
        cf = kName;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, "");
    }
    {
        cf = kSize;
        QString ret = scene.d->classifierToActionID(cf);
        EXPECT_EQ(ret, "");
    }
}
class testMenuScene : public AbstractMenuScene
{
public:
    QString name() const { return "CanvasMenu"; }
};
TEST(ExtendCanvasScene, scene)
{
    stub_ext::StubExt stub;
    bool call = true;
    typedef AbstractMenuScene *(*fun_type)(QAction *);
    stub.set_lamda((fun_type)&AbstractMenuScene::scene, [&call](QAction *) {
        call = true;
        return nullptr;
    });
    ExtendCanvasScene scene;
    QAction action;
    EXPECT_NO_FATAL_FAILURE(scene.scene(&action));
    EXPECT_TRUE(call);
}
TEST(ExtendCanvasScene, create)
{
    stub_ext::StubExt stub;
    QMenu parent;
    ExtendCanvasScene scene;
    bool call = true;
    typedef AbstractMenuScene *(*fun_type)(QMenu *);
    stub.set_lamda((fun_type)&AbstractMenuScene::create, [&call](QMenu *) {
        call = true;
        return nullptr;
    });
    EXPECT_NO_FATAL_FAILURE(scene.create(&parent));
    EXPECT_TRUE(call);
}
TEST(ExtendCanvasScene, updateState)
{
    stub_ext::StubExt stub;
    QMenu parent;
    ExtendCanvasScene scene;
    bool call = true;
    typedef AbstractMenuScene *(*fun_type)(QMenu *);
    stub.set_lamda((fun_type)&AbstractMenuScene::updateState, [&call](QMenu *) {
        call = true;
        return nullptr;
    });
    EXPECT_NO_FATAL_FAILURE(scene.updateState(&parent));
    EXPECT_TRUE(call);
}
TEST(ExtendCanvasScene, actionFilter)
{
    stub_ext::StubExt stub;
    bool isShowError { false };
    stub.set_lamda(VADDR(DDialog, exec), [&isShowError] {
        isShowError = true;
        return 1;
    });

    RenameDialog::ModifyMode mode = RenameDialog::kReplace;
    stub.set_lamda(&RenameDialog::modifyMode, [&mode]() {
        return mode;
    });
    ExtendCanvasScene scene;
    testMenuScene testScene;
    QAction tempAction;
    tempAction.setProperty("actionID", "rename");
    EXPECT_FALSE(scene.actionFilter(&testScene, &tempAction));

    scene.d->onCollection = true;
    CollectionView v("uuid", nullptr);
    scene.d->view = &v;

    {
        mode = RenameDialog::kReplace;
        EXPECT_TRUE(scene.actionFilter(&testScene, &tempAction));
    }
    {
        mode = RenameDialog::kAdd;
        EXPECT_TRUE(scene.actionFilter(&testScene, &tempAction));
    }
    {
        mode = RenameDialog::kCustom;
        EXPECT_TRUE(scene.actionFilter(&testScene, &tempAction));
    }
}
