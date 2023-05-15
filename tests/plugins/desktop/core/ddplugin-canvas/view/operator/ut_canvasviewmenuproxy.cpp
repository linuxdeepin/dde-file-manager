// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/canvasviewmenuproxy.h"
#include "model/canvasproxymodel_p.h"
#include "model/fileinfomodel_p.h"
#include "view/canvasview_p.h"

#include "dfm-base/interfaces/abstractmenuscene.h"

#include "dfm-framework/dpf.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QVariantHash>
#include <QMenu>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_canvas;

TEST(CanvasViewMenuProxy, disableMenu)
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

    EXPECT_FALSE(CanvasViewMenuProxy::disableMenu());

    dis = true;
    EXPECT_TRUE(CanvasViewMenuProxy::disableMenu());
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
        EXPECT_EQ(params.value("DesktopGridPos").value<QPoint>(), QPoint(1,1));
        EXPECT_EQ(params.value("DesktopCanvasView").toLongLong(), (long)view);
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
    CanvasView *view = nullptr;
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

TEST(CanvasViewMenuProxy, showEmptyAreaMenu)
{
    CanvasView view;
    CanvasProxyModel model;
    view.setModel(&model);
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasProxyModel::rootUrl, [](){
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

    CanvasViewMenuProxy proxy(&view);
    proxy.showEmptyAreaMenu(Qt::NoItemFlags, QPoint(1,1));
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
        EXPECT_EQ(params.value("selectFiles").value<QList<QUrl>>().first(),
                  QUrl::fromLocalFile("/tmp/1"));
        EXPECT_EQ(params.value("DesktopGridPos").value<QPoint>(), QPoint(1,1));
        EXPECT_EQ(params.value("DesktopCanvasView").toLongLong(), (long)view);
        init = true;
        return true;
    }
};
}

TEST(CanvasViewMenuProxy, showNormalMenu)
{
    CanvasProxyModel model;
    FileInfoModel fmodel;
    fmodel.d->fileProvider->rootUrl = QUrl::fromLocalFile("/tmp");
    model.setSourceModel(&fmodel);

    CanvasView view;
    view.setModel(&model);

    CanvasSelectionModel sel(&model, nullptr);
    view.setSelectionModel(&sel);
    view.setModel(&model);

    auto in1 = QUrl::fromLocalFile("/tmp/1");
    model.d->fileList.append(in1);
    fmodel.d->fileList.append(in1);

    DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));

    model.d->fileMap.insert(in1, info1);
    fmodel.d->fileMap.insert(in1, info1);

    sel.selectedCache.append(model.index(0));

    stub_ext::StubExt stub;
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

//    stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &, QVariantHash))
//                   &EventChannelManager::push, [scene](EventChannelManager *, const QString &space,
//                   const QString &topic, QVariantHash param){

//        EXPECT_EQ(space, QString("dfmplugin_menu"));
//        EXPECT_EQ(topic, QString("slot_Menu_PerfectParams"));
//        return QVariant::fromValue(param);
//    });

    CanvasViewMenuProxy proxy(&view);
    proxy.showNormalMenu(model.index(0), Qt::NoItemFlags, QPoint(1,1));
    EXPECT_TRUE(TestEmptyScene::init);
    EXPECT_TRUE(TestEmptyScene::cr);
    EXPECT_TRUE(TestEmptyScene::up);
    EXPECT_TRUE(TestEmptyScene::tri);
    EXPECT_TRUE(exec);
}
