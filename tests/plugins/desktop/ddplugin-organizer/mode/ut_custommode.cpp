// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/custom/custommode_p.h"
#include "models/collectionmodel.h"
#include "config/configpresenter.h"
#include "interface/fileinfomodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmodelshell.h"
#include <dfm-base/dfm_desktop_defines.h>

#include <dfm-framework/event/eventchannel.h>
#include <QMimeData>
#include <QUuid>
#include <DFrame>
#include "stubext.h"
#include <gtest/gtest.h>

using namespace ddplugin_organizer;
using namespace dfmbase;
using namespace dpf;
DWIDGET_BEGIN_NAMESPACE
class UT_CustomMode : public testing::Test
{
public:
    virtual void SetUp() override
    {
        mode = new CustomMode;
    }
    virtual void TearDown() override
    {
        delete mode;
        stub.clear();
    }

    stub_ext::StubExt stub;
    CustomMode *mode = nullptr;
};

TEST_F(UT_CustomMode, initialize)
{
    CollectionModel m;
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    QList<CollectionBaseDataPtr> list;
    list.push_back(ptr);
    auto fun_type1 = static_cast<QList<CollectionBaseDataPtr> (ConfigPresenter::*)() const>(&ConfigPresenter::customProfile);
    stub.set_lamda(fun_type1, [&list]() {
        __DBG_STUB_INVOKE__
        return list;
    });

    auto fun_type2 = static_cast<FileInfoModelShell *(CollectionModel::*)() const>(&CollectionModel::modelShell);
    FileInfoModelShell shell;
    stub.set_lamda(fun_type2, [&shell]() {
        __DBG_STUB_INVOKE__
        return &shell;
    });

    stub.set_lamda(&CollectionModel::refresh, [](CollectionModel *, const QModelIndex &, bool, int, bool) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_TRUE(mode->initialize(&m));
    ptr.reset();
}

TEST_F(UT_CustomMode, layout)
{
    typedef CollectionStyle (*fun_type)(const QString &);
    stub.set_lamda((fun_type)(&ConfigPresenter::customStyle), [](const QString &) {
        __DBG_STUB_INVOKE__
        CollectionStyle res;
        return res;
    });

    auto fun_type1 = static_cast<CollectionStyle (CollectionHolder::*)() const>(&CollectionHolder::style);
    stub.set_lamda(fun_type1, []() {
        __DBG_STUB_INVOKE__
        CollectionStyle res;
        return res;
    });
    bool call = false;

    stub.set_lamda(&ConfigPresenter::writeCustomStyle, [&call](ConfigPresenter *self, const QList<CollectionStyle> &) {
        call = true;
        __DBG_STUB_INVOKE__
    });

    QString key = "temp_key";
    CollectionHolderPointer holder(new CollectionHolder("fake", nullptr));
    mode->d->holders.insert(key, holder);
    mode->layout();
    EXPECT_TRUE(call);
    holder.reset();
}

TEST_F(UT_CustomMode, rebuild)
{
    typedef QList<QUrl> (*fun_type)();
    QList<QUrl> res;
    res.push_back(QUrl("temp"));
    stub.set_lamda((fun_type)&CollectionModel::files, [&res]() {
        __DBG_STUB_INVOKE__
        return res;
    });
    stub.set_lamda(&CustomDataHandler::check, [](CustomDataHandler *self, const QSet<QUrl> &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ConfigPresenter::saveCustomProfile, [](ConfigPresenter *self, const QList<CollectionBaseDataPtr> &baseDatas) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&CollectionHolder::createFrame, [](CollectionHolder *self, Surface *surface, CollectionModel *model) {
        __DBG_STUB_INVOKE__
    });

    //    stub.set_lamda(&CollectionHolder::setCanvasModelShell,[](CollectionHolder*self, CanvasModelShell *){
    //        __DBG_STUB_INVOKE__
    //    });
    //    stub.set_lamda(&CollectionHolder::setCanvasViewShell,[](CollectionHolder*self, CanvasViewShell *){
    //        __DBG_STUB_INVOKE__
    //    });
    //    stub.set_lamda(&CollectionHolder::setCanvasGridShell,[](CollectionHolder*self, CanvasGridShell *){
    //        __DBG_STUB_INVOKE__
    //    });
    //    stub.set_lamda(&CollectionHolder::setCanvasManagerShell,[](CollectionHolder*self, CanvasManagerShell *){
    //        __DBG_STUB_INVOKE__
    //    });
    stub.set_lamda(&CollectionHolder::setName, [](CollectionHolder *self, const QString &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::setRenamable, [](CollectionHolder *self, const bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::setMovable, [](CollectionHolder *self, const bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::setAdjustable, [](CollectionHolder *self, const bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::setClosable, [](CollectionHolder *self, const bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::setStretchable, [](CollectionHolder *self, const bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CollectionHolder::show, [](CollectionHolder *self) {
        __DBG_STUB_INVOKE__
    });

    typedef void (*fun_type1)();
    stub.set_lamda((fun_type1)&CustomMode::layout, []() {
        __DBG_STUB_INVOKE__
    });

    bool call = false;
    QObject::connect(mode, &CustomMode::collectionChanged, mode, [&call]() {
        __DBG_STUB_INVOKE__
        call = true;
    });
    mode->d->dataHandler = new CustomDataHandler;
    CollectionBaseDataPtr ptr(new CollectionBaseData);
    ptr.value->name = QString("temp");
    ptr.value->items = QList { QUrl("temp_qurl") };
    mode->d->dataHandler->collections["windos"] = ptr;
    SurfacePointer surfPtr(new Surface);
    mode->surfaces.push_back(surfPtr);
    mode->rebuild();
    EXPECT_TRUE(call);
}

TEST_F(UT_CustomMode, onFileInserted)
{
    typedef QList<QUrl> (*fun_type)();
    QList<QUrl> res;
    stub.set_lamda((fun_type)&CollectionModel::files, [&res]() {
        __DBG_STUB_INVOKE__

        res.push_back(QUrl("temp1"));
        res.push_back(QUrl("temp2"));
        return res;
    });

    bool call = false;
    typedef bool (*fun_type1)(const QUrl &, QString &, int &);
    stub.set_lamda((fun_type1)(&CollectionDataProvider::takePreItem),
                   [](const QUrl &qurl, QString &key, int &index) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    typedef void (*fun_type2)(const QUrl &url, const QString &key, const int index);
    stub.set_lamda((fun_type2)&CustomDataHandler::insert,
                   [&call](const QUrl &url, const QString &key, const int index) {
                       call = true;
                       __DBG_STUB_INVOKE__
                   });
    QModelIndex parent;
    int first = 1;
    int last = 1;

    mode->d->dataHandler = new CustomDataHandler;
    mode->onFileInserted(parent, first, last);

    EXPECT_TRUE(call);
}

TEST_F(UT_CustomMode, filterDataRenamed)
{
    QUrl url1("url1");
    QUrl url2("url2");
    mode->d->dataHandler = new CustomDataHandler;
    EXPECT_FALSE(mode->filterDataRenamed(url1, url2));
}

TEST_F(UT_CustomMode, reset)
{
    mode->model = new CollectionModel();
    mode->d->dataHandler = new CustomDataHandler;
    EXPECT_NO_FATAL_FAILURE(mode->reset());
    delete mode->model;
    mode->model = nullptr;
}

TEST_F(UT_CustomMode, filterDataRested)
{
    QUrl url("temp");
    QList<QUrl> urls;
    urls.append(url);
    mode->d->dataHandler = new CustomDataHandler;
    CollectionBaseDataPtr ptr(new CollectionBaseData);
    ptr.value->items = QList { QUrl("temp") };

    mode->d->dataHandler->collections["window"] = ptr;

    EXPECT_TRUE(mode->filterDataRested(&urls));
}

TEST_F(UT_CustomMode, filterDropData)
{
    bool call = false;
    auto fun_type = static_cast<QPoint (CanvasViewShell::*)(const int &, const QPoint &)>(&CanvasViewShell::gridPos);
    stub.set_lamda(fun_type,
                   [&call](CanvasViewShell *self, const int &viewIndex, const QPoint &viewPoint) {
                       __DBG_STUB_INVOKE__
                       call = true;
                       QPoint res = QPoint();
                       return res;
                   });

    stub.set_lamda(&CollectionModel::take, [](CollectionModel *self, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    int viewIndex = 1;
    QPoint viewPoint = QPoint();
    QMimeData mimeData;
    mimeData.setUrls(QList { QUrl("temp") });

    CanvasGridShell GridShell;
    CanvasViewShell ViewShell;
    CanvasModelShell ModelShell;
    CollectionModel collection;

    mode->model = &collection;
    mode->canvasViewShell = &ViewShell;
    mode->canvasGridShell = &GridShell;
    mode->canvasModelShell = &ModelShell;
    CollectionBaseDataPtr ptr(new CollectionBaseData);
    ptr.value->items = QList { QUrl("temp") };
    ptr.value->key = QString("temp_key");
    mode->d->dataHandler = new CustomDataHandler;
    mode->d->dataHandler->collections["window"] = ptr;

    EXPECT_TRUE(mode->filterDropData(viewIndex, &mimeData, viewPoint, nullptr));
    mode->model = nullptr;
}

TEST_F(UT_CustomMode, onNewCollection)
{
    auto fun_type = static_cast<int (CanvasGridShell::*)(const QString &, QPoint *)>(&CanvasGridShell::point);
    stub.set_lamda(fun_type, [](CanvasGridShell *self, const QString &item, QPoint *pos) {
        __DBG_STUB_INVOKE__
        return 1;
    });

    bool call = false;
    stub.set_lamda(&ConfigPresenter::updateCustomStyle, [&call](ConfigPresenter *self, const CollectionStyle &) {
        __DBG_STUB_INVOKE__
        call = true;
    });

    stub.set_lamda(&CollectionModel::refresh,
                   [](CollectionModel *self, const QModelIndex &parent, bool global, int ms, bool file) {
                       __DBG_STUB_INVOKE__
                   });

    auto fun_type1 = static_cast<QModelIndex (CollectionModel::*)() const>(&CollectionModel::rootIndex);
    stub.set_lamda(fun_type1, []() {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });

    QUrl url("temp");
    QList list { url };

    CanvasGridShell GridShell;
    CanvasViewShell ViewShell;
    CollectionModel collection;

    mode->d->dataHandler = new CustomDataHandler;
    mode->canvasGridShell = &GridShell;
    mode->canvasViewShell = &ViewShell;
    mode->model = &collection;

    mode->onNewCollection(list);

    EXPECT_TRUE(call);
    for (auto it = mode->d->dataHandler->collections.begin();
         it != mode->d->dataHandler->collections.end();
         it++) {
        EXPECT_EQ((*it)->name, "New Collection");
        EXPECT_EQ((*it)->items, list);
    }
    mode->model = nullptr;
}

TEST_F(UT_CustomMode, onDeleteCollection)
{
    stub.set_lamda(&ConfigPresenter::saveCustomProfile, []() {
        __DBG_STUB_INVOKE__
    });

    Surface sur = Surface();
    sur.setProperty(DesktopFrameProperty::kPropScreenName, "test window");
    auto fun_type = static_cast<Surface *(CollectionHolder::*)() const>(&CollectionHolder::surface);
    stub.set_lamda(fun_type, [&sur]() {
        __DBG_STUB_INVOKE__
        return &sur;
    });

    bool call = false;
    DFrame frame = DFrame();
    frame.data->crect = QRect(0, 0, 1, 1);
    auto fun_type1 = static_cast<DFrame *(CollectionHolder::*)() const>(&CollectionHolder::frame);
    stub.set_lamda(fun_type1, [&frame, &call]() {
        __DBG_STUB_INVOKE__
        call = true;
        return &frame;
    });

    CanvasGridShell GridShell;
    CanvasViewShell ViewShell;
    CanvasModelShell ModelShell;
    CollectionModel collection;
    CustomDataHandler handler;
    mode->d->dataHandler = &handler;
    mode->canvasGridShell = &GridShell;
    mode->canvasViewShell = &ViewShell;
    mode->canvasModelShell = &ModelShell;
    mode->model = &collection;

    QString uuid = "uuid";
    CollectionHolderPointer holder_ptr(CollectionHolderPointer(new CollectionHolder(uuid, nullptr)));
    mode->d->holders["temp_key"] = holder_ptr;

    CollectionBaseDataPtr ptr(new CollectionBaseData);
    ptr.value->items = QList { QUrl("temp") };
    ptr.value->key = QString("temp_key");
    mode->d->dataHandler = new CustomDataHandler;
    mode->d->dataHandler->collections["window"] = ptr;

    mode->onDeleteCollection("temp_key");

    mode->model = nullptr;

    EXPECT_TRUE(call);
}

DWIDGET_END_NAMESPACE
