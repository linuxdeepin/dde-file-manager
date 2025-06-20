// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>
#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include "menu/canvasmenuscene_p.h"
#include "menu/canvasmenu_defines.h"
#include "model/canvasproxymodel.h"
#include "view/canvasview.h"
#include "view/canvasview_p.h"
#include "stubext.h"
#include <gtest/gtest.h>
#include "dfm-framework/dpf.h"
#include <QMenu>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusInterface>
using namespace dfmbase;
using namespace ddplugin_canvas;

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
QT_BEGIN_NAMESPACE
static const char *const kActionIconMenuSceneName = "ActionIconManager";
static const char *const kDConfigHiddenMenuSceneName = "DConfigMenuFilter";
static const char *const kCanvasBaseSortMenuSceneName = "CanvasBaseSortMenu";
static const char *const kNewCreateMenuSceneName = "NewCreateMenu";
static const char *const kTemplateMenuSceneName = "TemplateMenu";
static const char *const kClipBoardMenuSceneName = "ClipBoardMenu";
static const char *const kOpenWithMenuSceneName = "OpenWithMenu";
static const char *const kFileOperatorMenuSceneName = "FileOperatorMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kShareMenuSceneName = "ShareMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";
static const char *const kExtendMenuSceneName = "ExtendMenu";
static const char *const kOemMenuSceneName = "OemMenu";
static const char *const kBookmarkSceneName = "BookmarkMenu";
static const char *const kPorpertySceneName = "PropertyMenu";

class UT_CanvasMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menuScene = new CanvasMenuScene();
        menuSceneP = new CanvasMenuScenePrivate(menuScene);
    }
    virtual void TearDown() override
    {
        delete menuSceneP;
        delete menuScene;
        stub.clear();
    }
    CanvasMenuScene *menuScene = nullptr;
    CanvasMenuScenePrivate *menuSceneP = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasMenuScene, filterDisableAction)
{
    typedef AbstractMenuScene *(*fun_type)(QAction *);
    CanvasMenuScene menuS;
    stub.set_lamda((fun_type)(&CanvasMenuScene::scene), [&menuS](QAction *) {
        __DBG_STUB_INVOKE__
        return &menuS;
    });

    QMenu menu;
    QAction action1;
    QAction action2;
    QAction action3;
    action1.setProperty(ActionPropertyKey::kActionID, "ID_1");
    action2.setProperty(ActionPropertyKey::kActionID, "ID_2");
    action3.setProperty(ActionPropertyKey::kActionID, "ID_3");
    menu.insertAction(nullptr, &action1);
    menu.insertAction(nullptr, &action2);
    menu.insertAction(nullptr, &action3);

    menuSceneP->normalDisableActions.insert("CanvasMenu", "ID_1");
    menuSceneP->normalDisableActions.insert("CanvasMenu", "ID_2");
    menuSceneP->filterDisableAction(&menu);
    EXPECT_EQ(menu.actions().size(), 1);
    int isFind = menu.actions().indexOf(&action3);
    EXPECT_EQ(isFind, 0);

    menuSceneP->isEmptyArea = true;
    menu.insertAction(nullptr, &action1);
    menu.insertAction(nullptr, &action2);
    menuSceneP->emptyDisableActions.insert("CanvasMenu", "ID_1");
    menuSceneP->emptyDisableActions.insert("CanvasMenu", "ID_2");
    menuSceneP->emptyDisableActions.insert("CanvasMenu", "ID_3");
    menuSceneP->filterDisableAction(&menu);
    EXPECT_TRUE(menu.isEmpty());
}

TEST_F(UT_CanvasMenuScene, initialize)
{
    stub.set_lamda((QVariant(EventChannelManager::*)(const QString &, const QString &, QString))(&EventChannelManager::push),
                   [](EventChannelManager *self, const QString &str1, const QString &str2, QString str3) {
                       __DBG_STUB_INVOKE__
                       CanvasMenuScene *res = new CanvasMenuScene;

                       return QVariant::fromValue(res);
                   });

    bool call = false;
    typedef bool (*fun_type)(const QVariantHash &);
    stub.set_lamda((fun_type)(&AbstractMenuScene::initialize), [&call](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        call = true;
        return true;
    });

    QVariantHash params;
    params.insert(MenuParamKey::kCurrentDir, QUrl("currentDir_url"));
    QList<QUrl> urls;
    urls.push_back(QUrl("url1"));
    urls.push_back(QUrl("url2"));
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(urls));
    params.insert(MenuParamKey::kOnDesktop, true);
    params.insert(MenuParamKey::kIsEmptyArea, false);
    params.insert(MenuParamKey::kIndexFlags, Qt::ItemIsSelectable);
    params.insert(CanvasMenuParams::kDesktopGridPos, QPoint(1, 1));

    CanvasView view;
    params.insert(CanvasMenuParams::kDesktopCanvasView, (qlonglong)&view);
    menuScene->d->isEmptyArea = false;
    menuScene->initialize(params);

    EXPECT_EQ(menuScene->subscene().size(), 11);
    EXPECT_TRUE(call);

    params.insert(MenuParamKey::kIsEmptyArea, true);
    menuScene->initialize(params);
    EXPECT_EQ(menuScene->subscene().size(), 19);
}

TEST_F(UT_CanvasMenuScene, create)
{
    QMenu *parent;
    EXPECT_FALSE(menuScene->create(parent));
    parent = new QMenu;
    EXPECT_TRUE(menuScene->create(parent));
    delete parent;
}

TEST_F(UT_CanvasMenuScene, triggered)
{
    stub.set_lamda(&dfmbase::filterActionBySubscene, [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CanvasView::refresh, []() { __DBG_STUB_INVOKE__ });
    CanvasManager manager;
    CanvasManagerPrivate managerP(&manager);
    manager.d = &managerP;
    CanvasManagerHook hook;
    managerP.hookIfs = &hook;

    CanvasView canvas;
    menuScene->d->view = &canvas;
    CanvasProxyModel proxy;
    menuScene->d->view->setModel(&proxy);

    {
        QAction action;
        menuScene->d->predicateAction.insert("temp_action", &action);
        action.setProperty(ActionPropertyKey::kActionID, ActionID::kSrtName);
        EXPECT_TRUE(menuScene->triggered(&action));
    }

    {
        QAction action;
        menuScene->d->predicateAction.insert("temp_action", &action);
        menuScene->d->iconSizeAction.insert(&action, 1);
        EXPECT_TRUE(menuScene->triggered(&action));
        EXPECT_EQ(CanvasIns->iconLevel(), 1);
    }
    {
        QAction action;
        menuScene->d->predicateAction.insert("temp_action", &action);
        action.setProperty(ActionPropertyKey::kActionID, ActionID::kRefresh);
        EXPECT_TRUE(menuScene->triggered(&action));
    }
    {
        QAction action;
        menuScene->d->predicateAction.insert("temp_action", &action);
        action.setProperty(ActionPropertyKey::kActionID, ActionID::kWallpaperSettings);
        EXPECT_TRUE(menuScene->triggered(&action));
    }
}

TEST_F(UT_CanvasMenuScene, emptyMenu)
{
    QMenu parent;
    menuScene->emptyMenu(&parent);
    CanvasView view;
    menuScene->d->view = &view;
    CanvasProxyModel model;
    menuScene->d->view->setModel(&model);
}

class testItemView : public QAbstractItemView
{
    virtual QRect visualRect(const QModelIndex &index) const { return QRect(1, 1, 2, 2); }
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) {}
    virtual QModelIndex indexAt(const QPoint &point) const { return QModelIndex(); }
    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) { return QModelIndex(); }

    virtual int horizontalOffset() const { return 1; }
    virtual int verticalOffset() const { return 1; }

    virtual bool isIndexHidden(const QModelIndex &index) const { return true; }

    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {}
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const { return QRegion(1, 1, 2, 2); }
};

TEST_F(UT_CanvasMenuScene, sortBySubActions)
{
    QMenu parent;
    QMenu *res = menuScene->sortBySubActions(&parent);
    EXPECT_EQ(res->actions().size(), 4);
}
QT_END_NAMESPACE
