// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionview_p.h"
#include "view/collectionviewmenu.h"
#include "models/collectionmodel_p.h"
#include "delegate/collectionitemdelegate.h"
#include "interface/fileinfomodelshell.h"
#include "private/surface.h"

#include <dfm-base/dfm_global_defines.h>
#include "dfm-base/interfaces/abstractmenuscene.h"

#include "dfm-framework/dpf.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QMenu>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_organizer;

TEST(CollectionViewMenu, disableMenu)
{
    stub_ext::StubExt stub;
    bool dis = false;
    stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &, QVariantHash ))
                   &EventChannelManager::push, [&dis](EventChannelManager *, const QString &space,
                   const QString &topic, QVariantHash){

        EXPECT_EQ(space, QString("dfmplugin_menu"));
        EXPECT_EQ(topic, QString("slot_Menu_IsDisable"));
        return dis;
    });

    EXPECT_FALSE(CollectionViewMenu::disableMenu());

    dis = true;
    EXPECT_TRUE(CollectionViewMenu::disableMenu());
}

namespace  {
class TestEmptyScene : public AbstractMenuScene
{
public:
    TestEmptyScene() : AbstractMenuScene() {
        init = false;
        cr = false;
        up = false;
        tri = false;
    }
    QString name() const {return "";}
    bool initialize(const QVariantHash &params) {
        EXPECT_EQ(params.value("currentDir").toUrl(), QUrl::fromLocalFile("/tmp"));
        EXPECT_TRUE(params.value("onDesktop").toBool());
        EXPECT_EQ(params.value("windowId").toLongLong(), view->winId());
        EXPECT_TRUE(params.value("isEmptyArea").toBool());
        EXPECT_TRUE(params.value("OnColletion").toBool());
        EXPECT_EQ(params.value("ColletionView").toLongLong(), (qlonglong)view);
        EXPECT_EQ(params.value("DesktopCanvasView").toLongLong(), (qlonglong)wid);
        init = true;
        return true;
    }
    bool create(QMenu *parent) {
        menu = parent;
        ac = parent->addAction("test");
        cr = true;
        return true;
    }
    void updateState(QMenu *parent) {
        EXPECT_EQ(menu, parent);
        up = true;
    }
    bool triggered(QAction *action) {
        tri = true;
        EXPECT_EQ(ac, action);
        return true;
    }
    QMenu *menu = nullptr;
    QAction *ac = nullptr;
    CollectionView *view = nullptr;
    QWidget *wid;
    static bool init;
    static bool cr;
    static bool up;
    static bool tri;
};
bool TestEmptyScene::init = false;
bool TestEmptyScene::cr = false;
bool TestEmptyScene::up = false;
bool TestEmptyScene::tri = false;
}

TEST(CollectionViewMenu, showEmptyAreaMenu)
{
    CollectionView view("1", nullptr);
    CollectionModel model;
    view.QAbstractItemView::setModel(&model);
    stub_ext::StubExt stub;
    stub.set_lamda(&CollectionModel::fileUrl, [&model](CollectionModel *, const QModelIndex &index){
        EXPECT_EQ(index, model.rootIndex());
        return QUrl::fromLocalFile("/tmp");
    });

    stub.set_lamda(&CollectionModel::rootUrl, [](){
        return QUrl::fromLocalFile("/tmp");
    });

    TestEmptyScene *scene = new TestEmptyScene;
    scene->view = &view;
    bool exec = false;
    stub.set_lamda((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec, [&exec](
                   QMenu *self, const QPoint &, QAction *){
        exec = true;
        return self->actions().first();
    });

    stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &, QString))
                   &EventChannelManager::push, [scene](EventChannelManager *, const QString &space,
                   const QString &topic, QString name){

        EXPECT_EQ(space, QString("dfmplugin_menu"));
        EXPECT_EQ(topic, QString("slot_MenuScene_CreateScene"));
        EXPECT_EQ(name, QString("CanvasMenu"));
        return QVariant::fromValue(scene);
    });

    QWidget wid;
    scene->wid = &wid;
    stub.set_lamda(&CollectionViewMenu::getCanvasView, [&wid](){
        return &wid;
    });

    CollectionViewMenu proxy(&view);
    proxy.emptyAreaMenu();
    EXPECT_TRUE(TestEmptyScene::init);
    EXPECT_TRUE(TestEmptyScene::cr);
    EXPECT_TRUE(TestEmptyScene::up);
    EXPECT_TRUE(TestEmptyScene::tri);
    EXPECT_TRUE(exec);
}
namespace  {
class TestNormalScene : public TestEmptyScene
{
public:
    bool initialize(const QVariantHash &params) {
        EXPECT_EQ(params.value("currentDir").toUrl(), QUrl::fromLocalFile("/tmp"));
        EXPECT_TRUE(params.value("onDesktop").toBool());
        EXPECT_EQ(params.value("windowId").toLongLong(), view->winId());
        EXPECT_FALSE(params.value("isEmptyArea").toBool());
        QList<QUrl> urls = params.value("selectFiles").value<QList<QUrl>>();
        EXPECT_EQ(urls.size(), 2);
        if (urls.size() == 2) {
            EXPECT_EQ(urls.first(), QUrl::fromLocalFile("/tmp/2"));
            EXPECT_EQ(urls.last(), QUrl::fromLocalFile("/tmp/1"));
        }

        EXPECT_TRUE(params.value("OnColletion").toBool());
        EXPECT_EQ(params.value("ColletionView").toLongLong(), (qlonglong)view);
        EXPECT_EQ(params.value("DesktopCanvasView").toLongLong(), (qlonglong)wid);
        init = true;
        return true;
    }
};
}

TEST(CollectionViewMenu, showNormalMenu)
{
    CollectionModel model;
    FileInfoModelShell shell;
    model.d->shell = &shell;

    CollectionView view("", nullptr);
    view.QAbstractItemView::setModel(&model);

    QItemSelectionModel sel(&model, nullptr);
    view.setSelectionModel(&sel);

    auto in1 = QUrl::fromLocalFile("/tmp/1");
    model.d->fileList.append(in1);
    auto in2 = QUrl::fromLocalFile("/tmp/2");
    model.d->fileList.append(in2);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
    model.d->fileMap.insert(in1, info1);
    DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
    model.d->fileMap.insert(in2, info2);

    stub_ext::StubExt stub;
    stub.set_lamda(&QItemSelectionModel::selectedIndexes, [&model](){
        return QModelIndexList {model.index(0, 0), model.index(1, 0)};
    });
    stub.set_lamda(VADDR(CollectionView, isIndexHidden), [](){
        return false;
    });

    stub.set_lamda(&FileInfoModelShell::rootUrl, [](){
        return QUrl::fromLocalFile("/tmp");
    });

    TestNormalScene *scene = new TestNormalScene;
    scene->view = &view;
    bool exec = false;
    stub.set_lamda((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec, [&exec](
                   QMenu *self, const QPoint &, QAction *){
        exec = true;
        return self->actions().first();
    });

    stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &, QString))
                   &EventChannelManager::push, [scene](EventChannelManager *, const QString &space,
                   const QString &topic, QString name){

        EXPECT_EQ(space, QString("dfmplugin_menu"));
        EXPECT_EQ(topic, QString("slot_MenuScene_CreateScene"));
        EXPECT_EQ(name, QString("CanvasMenu"));
        return QVariant::fromValue(scene);
    });

    QWidget wid;
    scene->wid = &wid;
    stub.set_lamda(&CollectionViewMenu::getCanvasView, [&wid](){
        return &wid;
    });

    CollectionViewMenu proxy(&view);
    proxy.normalMenu(model.index(1, 0), Qt::NoItemFlags, QPoint(1,1));
    EXPECT_TRUE(TestEmptyScene::init);
    EXPECT_TRUE(TestEmptyScene::cr);
    EXPECT_TRUE(TestEmptyScene::up);
    EXPECT_TRUE(TestEmptyScene::tri);
    EXPECT_TRUE(exec);
}

TEST(CollectionViewMenu, getCanvasView)
{
    stub_ext::StubExt stub;
    QWidget root;
    root.setProperty("ScreenName", "1");

    stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &))&EventChannelManager::push,
                   [&root](EventChannelManager *, const QString &space, const QString &topic){
        EXPECT_EQ(space, QString("ddplugin_core"));
        EXPECT_EQ(topic, QString("slot_DesktopFrame_RootWindows"));
        return QVariant::fromValue(QList<QWidget *>{&root});
    });

    CollectionView view("", nullptr);
    CollectionViewMenu proxy(&view);
    EXPECT_EQ(proxy.getCanvasView(), nullptr);

    QWidget wid;
    view.setParent(&wid);
    EXPECT_EQ(proxy.getCanvasView(), nullptr);

    Surface sur;
    wid.setParent(&sur);

    EXPECT_EQ(proxy.getCanvasView(), nullptr);

    sur.setProperty("ScreenName", "1");
    EXPECT_EQ(proxy.getCanvasView(), nullptr);

    QWidget canvas(&root);

    EXPECT_EQ(proxy.getCanvasView(), nullptr);

    canvas.setProperty("WidgetName", "canvas");

    EXPECT_EQ(proxy.getCanvasView(), &canvas);

    wid.setParent(nullptr);
    view.setParent(nullptr);
    canvas.setParent(nullptr);
}
