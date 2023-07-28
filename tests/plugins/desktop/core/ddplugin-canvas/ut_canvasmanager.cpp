// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include "displayconfig.h"
#include "grid/canvasgrid.h"
#include "view/canvasview_p.h"
#include "model/canvasproxymodel_p.h"
#include "model/fileinfomodel_p.h"
#include "view/operator/fileoperatorproxy_p.h"

#include <dfm-base/dfm_desktop_defines.h>

#include "dfm-framework/dpf.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(CanvasManager, init)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    bool setModel = false;
    stub.set_lamda(&FileInfoModel::setRootUrl, [&setModel](){
        setModel = true;
        return QModelIndex();
    });

    obj.init();

    EXPECT_NE(obj.d->hookIfs, nullptr);
    EXPECT_NE(obj.d->broker, nullptr);
    EXPECT_NE(obj.d->viewHook, nullptr);
    EXPECT_NE(obj.d->sourceModelBroker, nullptr);
    EXPECT_NE(obj.d->modelBroker, nullptr);
    EXPECT_NE(obj.d->viewBroker, nullptr);
    EXPECT_NE(obj.d->gridBroker, nullptr);
    EXPECT_EQ(obj.d->global, &obj);

    EXPECT_NE(obj.d->selectionModel, nullptr);
    ASSERT_NE(obj.d->sourceModel, nullptr);
    ASSERT_NE(obj.d->canvasModel, nullptr);
    EXPECT_EQ(obj.d->canvasModel->sourceModel(), obj.d->sourceModel);
    EXPECT_EQ(obj.fileModel(), obj.d->sourceModel);
    EXPECT_EQ(obj.model(), obj.d->canvasModel);
    EXPECT_EQ(obj.selectionModel(), obj.d->selectionModel);
    EXPECT_EQ(obj.views(), obj.d->viewMap.values());
}

TEST(CanvasManager, update)
{
    CanvasManager obj;
    obj.d->viewMap.insert("1", CanvasViewPointer(new CanvasView));
    obj.d->viewMap.insert("2", CanvasViewPointer(new CanvasView));

    stub_ext::StubExt stub;
    QList<CanvasView *> upd;
    stub.set_lamda((void (CanvasView::*)())&CanvasView::update, [&upd](CanvasView *w){
        upd.append(w);
    });

    obj.update();
    ASSERT_EQ(obj.d->viewMap.size(), upd.size());
}
TEST(CanvasManager, iconLevel)
{
    CanvasManager obj;

    stub_ext::StubExt stub;
    bool call = false;
    stub.set_lamda(&DisplayConfig::iconLevel, [&call](){
       call = true;
       return 3;
    });

    EXPECT_EQ(obj.iconLevel(), 3);
    EXPECT_TRUE(call);

    CanvasViewPointer v1(new CanvasView());
    auto delegate1 = new CanvasItemDelegate(v1.get());
    v1->setItemDelegate(delegate1);
    obj.d->viewMap.insert("1", v1);
    stub.set_lamda(&CanvasItemDelegate::iconLevel, [](){
       return 9;
    });

    call = false;
    EXPECT_EQ(obj.iconLevel(), 9);
    EXPECT_FALSE(call);
}

TEST(CanvasManager, setIconLevel_noview)
{
    CanvasManager obj;
    obj.d->hookIfs = new CanvasManagerHook(&obj);

    stub_ext::StubExt stub;
    int isc = -1;
    // CanvasManagerHook::iconSizeChanged
    stub.set_lamda(((bool (EventDispatcherManager::*)(const QString &, const QString &, int))
                   &EventDispatcherManager::publish), [&isc]
                   (EventDispatcherManager *, const QString &t1, const QString &t2, int level) {
        if (t1 == "ddplugin_canvas") {
            if (t2 == "signal_CanvasManager_IconSizeChanged") {
                isc = level;
                return true;
            }
        }
        return false;
    });

    stub.set_lamda(&DisplayConfig::iconLevel, [](){
       return 3;
    });

    int level = -1;
    stub.set_lamda(&DisplayConfig::setIconLevel, [&level](DisplayConfig *, int lv){
       level = lv;
       return true;
    });

    obj.setIconLevel(3);
    EXPECT_EQ(isc, -1);
    EXPECT_EQ(level, -1);

    level = -1;
    isc = -1;
    obj.setIconLevel(1);
    EXPECT_EQ(isc, 1);
    EXPECT_EQ(level, 1);
}

TEST(CanvasManager, setIconLevel_withview)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    int isc = -1;
    // CanvasManagerHook::iconSizeChanged
    stub.set_lamda(((bool (EventDispatcherManager::*)(const QString &, const QString &, int))
                   &EventDispatcherManager::publish), [&isc]
                   (EventDispatcherManager *, const QString &t1, const QString &t2, int level) {
        if (t1 == "ddplugin_canvas") {
            if (t2 == "signal_CanvasManager_IconSizeChanged") {
                isc = level;
                return true;
            }
        }
        return false;
    });

    stub.set_lamda(&DisplayConfig::iconLevel, [](){
       return 3;
    });

    int level = -1;
    stub.set_lamda(&DisplayConfig::setIconLevel, [&level](DisplayConfig *, int lv){
       level = lv;
       return true;
    });

    int updateGrid = 0;
    stub.set_lamda(&CanvasView::updateGrid, [&updateGrid](){
        updateGrid++;
    });

    int dellv = 0;
    stub.set_lamda(&CanvasItemDelegate::setIconLevel, [&dellv](CanvasItemDelegate *, int l){
        dellv += l;
        return l;
    });
    stub.set_lamda(&CanvasItemDelegate::iconLevel, [](){
       return 3;
    });

    CanvasViewPointer v1(new CanvasView());
    auto delegate1 = new CanvasItemDelegate(v1.get());
    v1->setItemDelegate(delegate1);
    obj.d->viewMap.insert("1", v1);

    CanvasViewPointer v2(new CanvasView());
    auto delegate2 = new CanvasItemDelegate(v1.get());
    v2->setItemDelegate(delegate2);
    obj.d->viewMap.insert("2", v2);

    obj.d->hookIfs = new CanvasManagerHook(&obj);

    {
        isc = -1;
        level = -1;
        updateGrid = 0;
        dellv = 0;
        obj.setIconLevel(-1);
        EXPECT_EQ(isc, -1);
        EXPECT_EQ(level, -1);
        EXPECT_EQ(updateGrid, 0);
        EXPECT_EQ(dellv, 0);
    }

    {
        isc = -1;
        level = -1;
        updateGrid = 0;
        dellv = 0;
        obj.setIconLevel(5);
        EXPECT_EQ(isc, -1);
        EXPECT_EQ(level, -1);
        EXPECT_EQ(updateGrid, 0);
        EXPECT_EQ(dellv, 0);
    }

    {
        isc = -1;
        level = -1;
        updateGrid = 0;
        dellv = 0;
        obj.setIconLevel(3);
        EXPECT_EQ(isc, -1);
        EXPECT_EQ(level, -1);
        EXPECT_EQ(updateGrid, 0);
        EXPECT_EQ(dellv, 0);
    }

    {
        isc = -1;
        level = -1;
        updateGrid = 0;
        dellv = 0;
        obj.setIconLevel(4);
        EXPECT_EQ(isc, 4);
        EXPECT_EQ(level, 4);
        EXPECT_EQ(updateGrid, 2);
        EXPECT_EQ(dellv, 8);
    }
}

TEST(CanvasManager, autoArrange)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    bool ret = false;
    stub.set_lamda(&DisplayConfig::autoAlign, [&ret](){
        return ret;
    });

    EXPECT_EQ(ret, obj.autoArrange());

    ret = true;
    EXPECT_EQ(ret, obj.autoArrange());
}

TEST(CanvasManager, refresh)
{
    CanvasManager obj;
    CanvasViewPointer vp(new CanvasView());
    obj.d->viewMap.insert("test", vp);

    stub_ext::StubExt stub;
    bool silent = false;
    stub.set_lamda(&CanvasView::refresh,[&silent](CanvasView *self, bool s){
        silent = s;
    });

    obj.refresh(true);
    EXPECT_TRUE(silent);

    silent = true;
    obj.refresh(false);
    EXPECT_FALSE(false);
}

TEST(CanvasManager, setAutoArrange)
{
    CanvasManager obj;
    obj.d->hookIfs = new CanvasManagerHook(&obj);

    stub_ext::StubExt stub;
    bool isOn = false;
    stub.set_lamda(&DisplayConfig::setAutoAlign, [&isOn](DisplayConfig *, bool on){
        isOn = on;
    });

    CanvasGrid::Mode mode = CanvasGrid::Mode::Custom;
    stub.set_lamda(&CanvasGrid::setMode, [&mode](CanvasGrid *, CanvasGrid::Mode m){
        mode = m;
    });

    bool call = false;
    stub.set_lamda(&CanvasGrid::arrange, [&call]() {
        call = true;
    });

    bool notify = false;
    // CanvasManagerHook::autoArrangeChanged
    stub.set_lamda(((bool (EventDispatcherManager::*)(const QString &, const QString &, bool))
                   &EventDispatcherManager::publish), [&notify]
                   (EventDispatcherManager *, const QString &t1, const QString &t2, bool au) {
        if (t1 == "ddplugin_canvas") {
            if (t2 == "signal_CanvasManager_AutoArrangeChanged") {
                notify = au;
                return true;
            }
        }
        return false;
    });

    obj.setAutoArrange(true);
    EXPECT_TRUE(isOn);
    EXPECT_EQ(mode, CanvasGrid::Mode::Align);
    EXPECT_TRUE(call);
    EXPECT_TRUE(notify);

    isOn = true;
    mode = CanvasGrid::Mode::Align;
    call = false;
    notify = true;
    obj.setAutoArrange(false);
    EXPECT_FALSE(isOn);
    EXPECT_EQ(mode, CanvasGrid::Mode::Custom);
    EXPECT_FALSE(call);
    EXPECT_FALSE(notify);
}

TEST(CanvasManager, onDetachWindows)
{
    CanvasManager obj;
    QWidget wid;
    CanvasViewPointer v1(new CanvasView(&wid));
    obj.d->viewMap.insert("1", v1);

    obj.onDetachWindows();

    EXPECT_EQ(v1->parent(), nullptr);
}

TEST(CanvasManager, onCanvasBuild2)
{
    stub_ext::StubExt stub;
    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "1");

    QList<QWidget *> roots { &root };
    stub.set_lamda(((QVariant (EventChannelManager::*)(const QString &, const QString &))
                   &EventChannelManager::push), [&roots]
                   (EventChannelManager *, const QString &t1, const QString &t2) {
        if (t1 == "ddplugin_core") {
            if (t2 == "slot_DesktopFrame_RootWindows") {
                return QVariant::fromValue(roots);
            }
        }

        return QVariant();
    });

    bool uv = false;
    stub.set_lamda(&CanvasManagerPrivate::updateView, [&uv](){
        uv = true;
    });

    stub.set_lamda(&CanvasManagerPrivate::createView, [](){
        return CanvasViewPointer(new CanvasView());
    });

    int surface = 0;
    stub.set_lamda(&CanvasGrid::initSurface, [&surface](CanvasGrid *, int c){
        surface = c;
    });

    bool reload = false;
    stub.set_lamda(&CanvasManager::reloadItem, [&reload](){
        reload = true;
    });

    CanvasManager obj;
    obj.d->sourceModel = new FileInfoModel(&obj);
    int st = 0;
    stub.set_lamda(&FileInfoModel::modelState, [&st](){
        return st;
    });

    {
        uv = false;
        reload = false;
        surface = -1;
        obj.onCanvasBuild();
        EXPECT_EQ(surface, 1);
        EXPECT_FALSE(uv);
        EXPECT_FALSE(reload);
        EXPECT_TRUE(obj.d->viewMap.contains("1"));
    }

    {
        uv = false;
        reload = false;
        surface = -1;
        st = 1;
        obj.onCanvasBuild();
        EXPECT_EQ(surface, 1);
        EXPECT_TRUE(uv);
        EXPECT_TRUE(reload);
        EXPECT_EQ(obj.d->viewMap.size(), 1);
        EXPECT_TRUE(obj.d->viewMap.contains("1"));
    }

    QWidget root2;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "0");
    root2.setProperty(DesktopFrameProperty::kPropScreenName, "2");
    roots = { &root, &root2};

    {
        uv = false;
        reload = false;
        surface = -1;
        st = 1;
        obj.onCanvasBuild();
        EXPECT_EQ(surface, 2);
        EXPECT_FALSE(uv);
        EXPECT_TRUE(reload);
        EXPECT_EQ(obj.d->viewMap.size(), 2);
        EXPECT_FALSE(obj.d->viewMap.contains("1"));
        EXPECT_TRUE(obj.d->viewMap.contains("0"));
        EXPECT_TRUE(obj.d->viewMap.contains("2"));
    }

    root.setProperty(DesktopFrameProperty::kPropScreenName, "1");
    {
        uv = false;
        reload = false;
        surface = -1;
        st = 1;
        obj.onCanvasBuild();
        EXPECT_EQ(surface, 2);
        EXPECT_TRUE(uv);
        EXPECT_TRUE(reload);
        EXPECT_EQ(obj.d->viewMap.size(), 2);
        EXPECT_TRUE(obj.d->viewMap.contains("1"));
        EXPECT_FALSE(obj.d->viewMap.contains("0"));
        EXPECT_TRUE(obj.d->viewMap.contains("2"));
    }
}

TEST(CanvasManager, onWallperSetting)
{
    CanvasManager obj;
    obj.d->hookIfs = new CanvasManagerHook(&obj);
    stub_ext::StubExt stub;

    QString emited;
    stub.set_lamda(((bool (EventSequenceManager::*)(const QString &, const QString &, QString))
                   &EventSequenceManager::run), [&emited]
                   (EventSequenceManager *, const QString &t1, const QString &t2, QString screen) {
        if (t1 == "ddplugin_canvas") {
            if (t2 == "hook_CanvasManager_RequestWallpaperSetting") {
                emited = screen;
                return true;
            }
        }
        return false;
    });

    obj.onWallperSetting(nullptr);
    EXPECT_TRUE(emited.isEmpty());

    emited.clear();

    CanvasViewPointer v1(new CanvasView());
    obj.d->viewMap.insert("1", v1);

    obj.onWallperSetting(v1.get());
    EXPECT_EQ(emited, QString("1"));
}

TEST(CanvasManager, reloadItem)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    obj.d->canvasModel = new CanvasProxyModel(&obj);
    stub.set_lamda(&CanvasProxyModel::files, [](){
        return QList<QUrl>{QUrl::fromLocalFile("/home"),QUrl::fromLocalFile("/var")};
    });

    QStringList items;
    stub.set_lamda(&CanvasGrid::setItems, [&items](CanvasGrid *, const QStringList &item){
        items = item;
    });

    bool isAligin = false;
    stub.set_lamda(&DisplayConfig::autoAlign, [&isAligin](){
        return isAligin;
    });

    bool arrange = false;
    stub.set_lamda(&CanvasGrid::arrange, [&arrange](){
        arrange = true;
    });

    obj.reloadItem();
    EXPECT_EQ(items.size(), 2);
    EXPECT_EQ(GridIns->mode(), CanvasGrid::Mode::Custom);
    EXPECT_FALSE(arrange);

    items.clear();
    isAligin = true;
    obj.reloadItem();
    EXPECT_EQ(items.size(), 2);
    EXPECT_EQ(GridIns->mode(), CanvasGrid::Mode::Align);
    EXPECT_TRUE(arrange);
}

TEST(CanvasManager, onChangeIconLevel)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    int lv = -1;
    stub.set_lamda(&CanvasManager::setIconLevel, [&lv](CanvasManager *, int l){
        lv = l;
    });

    obj.onChangeIconLevel(true);
    EXPECT_EQ(lv, -1);

    obj.onChangeIconLevel(false);
    EXPECT_EQ(lv, -1);

    CanvasViewPointer v2(new CanvasView());
    auto delegate2 = new CanvasItemDelegate(v2.get());
    v2->setItemDelegate(delegate2);
    obj.d->viewMap.insert("1", v2);

    stub.set_lamda(&CanvasItemDelegate::iconLevel, [](){
        return 2;
    });

    lv = -1;
    obj.onChangeIconLevel(true);
    EXPECT_EQ(lv, 3);

    lv = -1;
    obj.onChangeIconLevel(false);
    EXPECT_EQ(lv, 1);
}

TEST(CanvasManagerPrivate, createView)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    obj.d->canvasModel = new CanvasProxyModel(&obj);
    obj.d->selectionModel = new CanvasSelectionModel(obj.d->canvasModel, &obj);

    bool init = false;
    stub.set_lamda(&CanvasView::initUI, [&init](){
        init = true;
    });

    bool updateGrid = false;
    stub.set_lamda(&CanvasView::updateGrid, [&updateGrid](){
        updateGrid = true;
    });

    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "0");
    root.setProperty(DesktopFrameProperty::kPropScreenAvailableGeometry, QRect(900, 50, 900, 850));
    root.setProperty(DesktopFrameProperty::kPropScreenGeometry, QRect(900, 0, 900, 900));

    {
        EXPECT_EQ(obj.d->createView(nullptr, 0), nullptr);
        EXPECT_EQ(obj.d->createView(&root, 0), nullptr);
    }

    {
        init = false;
        updateGrid = false;
        auto view = obj.d->createView(&root, 1);
        ASSERT_NE(view, nullptr);
        EXPECT_TRUE(init);
        EXPECT_TRUE(updateGrid);
        EXPECT_EQ(view->parent(), &root);
        EXPECT_EQ(view->model(), obj.d->canvasModel);
        EXPECT_EQ(view->selectionModel(), obj.d->selectionModel);
        EXPECT_EQ(view->screenNum(), 1);
        EXPECT_EQ(view->geometry(), QRect(0, 50, 900, 850));
        EXPECT_EQ(view->property(DesktopFrameProperty::kPropScreenName).toString(), "0");
        EXPECT_EQ(view->property(DesktopFrameProperty::kPropWidgetName).toString(), "canvas");
        EXPECT_EQ(view->property(DesktopFrameProperty::kPropWidgetLevel).toReal(), 10.0);
    }
}

TEST(CanvasManagerPrivate, updateView)
{
    CanvasManager obj;
    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "0");
    root.setProperty(DesktopFrameProperty::kPropScreenAvailableGeometry, QRect(900, 50, 900, 850));
    root.setProperty(DesktopFrameProperty::kPropScreenGeometry, QRect(900, 0, 900, 900));

    stub_ext::StubExt stub;
    bool updateGrid = false;
    stub.set_lamda(&CanvasView::updateGrid, [&updateGrid](){
        updateGrid = true;
    });

    CanvasViewPointer view;
    view.reset(new CanvasView());

    obj.d->updateView(view, &root, 2);
    EXPECT_EQ(view->parent(), &root);
    EXPECT_EQ(view->screenNum(), 2);
    EXPECT_EQ(view->geometry(), QRect(0, 50, 900, 850));
    EXPECT_EQ(view->property(DesktopFrameProperty::kPropScreenName).toString(), "0");
    EXPECT_TRUE(updateGrid);
}

TEST(CanvasManagerPrivate, onFileSorted)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    obj.d->canvasModel = new CanvasProxyModel(&obj);
    stub.set_lamda(&CanvasProxyModel::files, [](){
        return QList<QUrl>{QUrl::fromLocalFile("/home"),QUrl::fromLocalFile("/var")};
    });

    QStringList items;
    CanvasGrid::Mode m = CanvasGrid::Mode::Custom;
    GridIns->setMode(m);
    stub.set_lamda(&CanvasGrid::setItems, [&items, &m](CanvasGrid *self, const QStringList &item){
        items = item;
        m = self->d->mode;
    });

    obj.d->onFileSorted();

    EXPECT_EQ(items.size(), 2);
    EXPECT_EQ(m, CanvasGrid::Mode::Align);
    EXPECT_EQ(GridIns->mode(), CanvasGrid::Mode::Custom);
}

TEST(CanvasManagerPrivate, onHiddenFlagsChanged)
{
    CanvasManager obj;
    stub_ext::StubExt stub;
    obj.d->canvasModel = new CanvasProxyModel(&obj);

    bool flag = false;
    stub.set_lamda(&CanvasProxyModel::showHiddenFiles, [&flag](){
        return flag;
    });

    bool sflag = false;
    stub.set_lamda(&CanvasProxyModel::setShowHiddenFiles, [&sflag](CanvasProxyModel *, bool f){
        sflag = f;
    });

    QModelIndex ridx;
    stub.set_lamda(&CanvasProxyModel::refresh, [&ridx](CanvasProxyModel *, const QModelIndex &parent, bool global, int ms, bool file){
        ridx = parent;
        EXPECT_FALSE(global);
        EXPECT_GT(ms, 0);
    });

    obj.d->onHiddenFlagsChanged(false);
    EXPECT_FALSE(ridx.isValid());

    {
        flag = false;
        ridx = QModelIndex();
        sflag = false;
        obj.d->onHiddenFlagsChanged(true);
        EXPECT_TRUE(sflag);
        EXPECT_EQ(ridx, obj.d->canvasModel->rootIndex());
    }

    {
        flag = true;
        ridx = QModelIndex();
        sflag = true;
        obj.d->onHiddenFlagsChanged(false);
        EXPECT_FALSE(sflag);
        EXPECT_EQ(ridx, obj.d->canvasModel->rootIndex());
    }
}

class TestCanvasMrgFile : public testing::Test
{
public:
    TestCanvasMrgFile() : Test(){}
    void SetUp() override {
        obj.d->sourceModel = new FileInfoModel(&obj);
        obj.d->canvasModel = new CanvasProxyModel(&obj);
        obj.d->canvasModel->setSourceModel(obj.d->sourceModel);
        {
            obj.d->sourceModel->d->fileList.append(fakeFile);
            fakeInfo.reset(new SyncFileInfo(fakeFile));
            obj.d->sourceModel->d->fileMap.insert(fakeFile, fakeInfo);

            obj.d->canvasModel->d->fileList.append(fakeFile);
            obj.d->canvasModel->d->fileMap.insert(fakeFile, fakeInfo);
        }

        obj.d->selectionModel = new CanvasSelectionModel(obj.d->canvasModel, &obj);

        CanvasViewPointer v2(new CanvasView());
        auto delegate2 = new CanvasItemDelegate(v2.get());
        v2->setItemDelegate(delegate2);
        v2->setModel(obj.d->canvasModel);
        v2->setSelectionModel(obj.d->selectionModel);

        obj.d->viewMap.insert("1", v2);
    }

    CanvasManager obj;
    stub_ext::StubExt stub;
    QUrl fakeFile = QUrl::fromLocalFile("/home/test222");
    DFMSyncFileInfoPointer fakeInfo;
};

TEST_F(TestCanvasMrgFile, onFileRenamed)
{
    QModelIndex cidx;
    QUrl old = QUrl::fromLocalFile("/home/test");
    stub.set_lamda(&CanvasView::setCurrentIndex, [&cidx](QAbstractItemView *, const QModelIndex &index){
        cidx = index;
    });
    FileOperatorProxyIns->d->renameFileData.insert(old, fakeFile);

    bool call = false;
    stub.set_lamda(&CanvasGrid::replace, [&call](){
        call = true;
        return true;
    });

    obj.d->onFileRenamed(old, fakeFile);

    ASSERT_TRUE(cidx.isValid());
    EXPECT_EQ(cidx, obj.d->canvasModel->index(fakeFile));
    EXPECT_TRUE(obj.d->selectionModel->isSelected(cidx));
    EXPECT_TRUE(call);
    EXPECT_TRUE(FileOperatorProxyIns->d->renameFileData.isEmpty());
}

TEST_F(TestCanvasMrgFile, onFileInserted)
{
    QString callAppend;
    stub.set_lamda((void (CanvasGrid::*)(const QString &))&CanvasGrid::append, [&callAppend](CanvasGrid *, const QString &item) {
        callAppend = item;
    });

    bool hasPoint = false;
    stub.set_lamda(&CanvasGrid::point, [&hasPoint]() {
        return hasPoint;
    });

    QString item;
    int index = -1;
    QPoint begin;
    stub.set_lamda(&CanvasGrid::tryAppendAfter, [&item, &index, &begin](CanvasGrid *,
                   const QStringList &its, int idx, const QPoint &b) {
        item = its.first();
        index = idx;
        begin = b;
    });

    QUrl open;
    stub.set_lamda(&CanvasManager::openEditor, [&open](CanvasManager *, const QUrl &url) {
        open = url;
    });

    {
        hasPoint = true;
        FileOperatorProxyIns->d->touchFileData = qMakePair(QString(), qMakePair(-1, QPoint(-1, -1)));
        GridIns->setMode(CanvasGrid::Mode::Align);

        obj.d->onFileInserted(obj.d->canvasModel->rootIndex(), 0, 0);
        EXPECT_TRUE(callAppend.isEmpty());
        EXPECT_TRUE(open.isEmpty());
        EXPECT_TRUE(item.isEmpty());
    }

    {
        callAppend.clear();
        item.clear();
        hasPoint = false;
        FileOperatorProxyIns->d->touchFileData = qMakePair(QString(), qMakePair(-1, QPoint(-1, -1)));
        GridIns->setMode(CanvasGrid::Mode::Align);

        obj.d->onFileInserted(obj.d->canvasModel->rootIndex(), 0, 0);
        EXPECT_EQ(callAppend, fakeFile.toString());
        EXPECT_TRUE(open.isEmpty());
        EXPECT_TRUE(item.isEmpty());
    }

    // touch
    {
        callAppend.clear();
        hasPoint = false;
        FileOperatorProxyIns->d->touchFileData = qMakePair(fakeFile.toString(), qMakePair(1, QPoint(1, 1)));
        GridIns->setMode(CanvasGrid::Mode::Align);

        obj.d->onFileInserted(obj.d->canvasModel->rootIndex(), 0, 0);
        EXPECT_EQ(callAppend, fakeFile.toString());
        EXPECT_EQ(open, fakeFile);

        EXPECT_TRUE(item.isEmpty());
        EXPECT_TRUE(FileOperatorProxyIns->d->touchFileData.first.isEmpty());
    }

    // touch2
    {
        callAppend.clear();
        hasPoint = false;
        item.clear();
        index = -1;
        begin = QPoint();
        FileOperatorProxyIns->d->touchFileData = qMakePair(fakeFile.toString(), qMakePair(1, QPoint(1, 1)));
        GridIns->setMode(CanvasGrid::Mode::Custom);

        obj.d->onFileInserted(obj.d->canvasModel->rootIndex(), 0, 0);
        EXPECT_TRUE(callAppend.isEmpty());
        EXPECT_EQ(open, fakeFile);
        EXPECT_EQ(index, 1);
        EXPECT_EQ(begin, QPoint(1, 1));

        EXPECT_TRUE(FileOperatorProxyIns->d->touchFileData.first.isEmpty());
        EXPECT_EQ(item, fakeFile.toString());
    }
}

TEST_F(TestCanvasMrgFile, onFileAboutToBeRemoved)
{
    bool hasPoint = false;
    stub.set_lamda(&CanvasGrid::point, [&hasPoint,this](CanvasGrid *, const QString &item, QPair<int, QPoint> &pos) {
        EXPECT_EQ(item, fakeFile.toString());
        pos.first = 2;
        return hasPoint;
    });

    int ridx = -1;
    QString ritem;
    stub.set_lamda(&CanvasGrid::remove, [&hasPoint, &ridx, &ritem](CanvasGrid *, int index, const QString &item) {
        ridx = index;
        ritem = item;
        return true;
    });

    bool overload = false;
    stub.set_lamda(&CanvasGrid::overloadItems, [&overload, this]() {
        overload = true;
        return QStringList{fakeFile.toString()};
    });

    bool arrange = false;
    stub.set_lamda(&CanvasGrid::arrange, [&arrange]() {
        arrange = true;
    });

    bool popOverload = false;
    stub.set_lamda(&CanvasGrid::popOverload, [&popOverload]() {
        popOverload = true;
    });

    // no point
    {
        hasPoint = false;
        obj.d->onFileAboutToBeRemoved(obj.d->canvasModel->rootIndex(), 0, 0);
        EXPECT_EQ(ridx, 1);
        EXPECT_EQ(ritem, fakeFile.toString());
        EXPECT_TRUE(overload);
        EXPECT_FALSE(popOverload);
        EXPECT_FALSE(arrange);
    }

    // align
    {
        hasPoint = true;
        popOverload = false;
        arrange = false;
        overload = false;
        GridIns->setMode(CanvasGrid::Mode::Align);
        obj.d->onFileAboutToBeRemoved(obj.d->canvasModel->rootIndex(), 0, 0);

        EXPECT_EQ(ridx, 2);
        EXPECT_EQ(ritem, fakeFile.toString());
        EXPECT_FALSE(overload);
        EXPECT_FALSE(popOverload);
        EXPECT_TRUE(arrange);
    }

    // custom
    {
        hasPoint = true;
        popOverload = false;
        arrange = false;
        overload = false;
        GridIns->setMode(CanvasGrid::Mode::Custom);
        obj.d->onFileAboutToBeRemoved(obj.d->canvasModel->rootIndex(), 0, 0);

        EXPECT_EQ(ridx, 2);
        EXPECT_EQ(ritem, fakeFile.toString());
        EXPECT_FALSE(overload);
        EXPECT_TRUE(popOverload);
        EXPECT_FALSE(arrange);
    }
}
