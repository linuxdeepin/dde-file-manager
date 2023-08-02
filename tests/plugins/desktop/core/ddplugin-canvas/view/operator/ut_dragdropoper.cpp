// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/canvasview_p.h"
#include "view/operator/dragdropoper.h"
#include "model/canvasproxymodel.h"
#include <dfm-base/utils/fileutils.h>
#include "model/canvasmodelfilter.h"
#include "model/canvasproxymodel_p.h"
#include <DFileDragClient>
#include <dfm-base/interfaces/fileinfo.h>
#include <gtest/gtest.h>
#include "core/ddplugin-canvas/utils/keyutil.h"
#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include <QMimeData>
#include <QDropEvent>
#include "dfm-base/interfaces/fileinfo.h"
#include <DFileDragClient>
#include <qcoreevent.h>
#include <dfm-base/mimedata/dfmmimedata.h>
using namespace testing;
using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE
DGUI_USE_NAMESPACE
class DragDropOperTest : public Test
{
public:
    void SetUp() override {
       global =new CanvasManager;
    }
    void TearDown() override {
        delete global;
        stub.clear();
    }
    CanvasManager *global;

    stub_ext::StubExt stub;
};

TEST_F(DragDropOperTest, testEnter) {

    //  Prohibit drag  special directory
    CanvasView view;
    DragDropOper opt(&view);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    CanvasProxyModel *md = new CanvasProxyModel(nullptr);

    bool updateHover = false;
    stub.set_lamda(&DragDropOper::updateDragHover, [&updateHover](){
        updateHover = true;
        return;
    });

    bool updateMime = false;
    stub.set_lamda(&DragDropOper::updateDFMMimeData, [&updateMime](){
        updateMime = true;
        return;
    });

    // action1
    stub.set_lamda(&DragDropOper::checkProhibitPaths, []() -> bool{
        return true;
    });

    EXPECT_TRUE(opt.enter(&event));
    EXPECT_TRUE(updateHover);
    EXPECT_TRUE(updateMime);

    stub.reset(&DragDropOper::checkProhibitPaths);

    // action2
    stub.set_lamda(&DragDropOper::checkProhibitPaths, []() -> bool{
        return false;
    });

    stub.set_lamda(&CanvasView::model, [md]() -> CanvasProxyModel* {
        return md;
    });

    stub.set_lamda(&CanvasProxyModel::rootUrl, []() -> QUrl {
        return QUrl();
    });

    stub.set_lamda(&DFileDragClient::checkMimeData, []() -> bool {
        return true;
    });

    stub.set_lamda(&DFileDragClient::setTargetUrl, []() {
            return;
        });

    Qt::DropAction act = Qt::IgnoreAction;
    bool isCallUpdate = false;

    stub.set_lamda(&QDropEvent::setDropAction, [&]() {
        return;
    });

    stub.set_lamda(&DragDropOper::updatePrepareDodgeValue, [&]() {
        isCallUpdate = true;
        return;
    });

    EXPECT_EQ(true, opt.enter(&event));
    EXPECT_TRUE(!isCallUpdate);
    stub.reset(&DFileDragClient::checkMimeData);


    // action3
    stub.set_lamda(&DFileDragClient::checkMimeData, []() -> bool {
        return false;
    });

    stub.set_lamda(&DragDropOper::checkXdndDirectSave, []() -> bool {
        return true;
    });

    opt.enter(&event);
    EXPECT_TRUE(isCallUpdate);
    EXPECT_EQ(true, opt.enter(&event));
    stub.reset(&DragDropOper::checkXdndDirectSave);

    // action4
    stub.set_lamda(&DragDropOper::checkXdndDirectSave, []() -> bool {
        return false;
    });

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
                       return {};
                   });

    typedef void (*func)(DragDropOper*,QDropEvent *event, const QList<QUrl> &urls, const QUrl &targetFileUrl);
    auto testFunc = (func)(&DragDropOper::preproccessDropEvent);
    stub.set_lamda(testFunc, []() {
        return;
    });

    EXPECT_TRUE(isCallUpdate);
    EXPECT_EQ(false, opt.enter(&event));
    delete md;
    md = nullptr;
}

TEST_F(DragDropOperTest, testCheckProhibitPaths) {
    DFMBASE_USE_NAMESPACE

    DragDropOper opt(nullptr);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return {};
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return false;
    });
    EXPECT_FALSE(opt.checkProhibitPaths(&event));
    stub.reset(&QMimeData::urls);

    //action2

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });
    EXPECT_EQ(false, opt.checkProhibitPaths(&event));
    stub.reset(&QMimeData::urls);
    stub.reset(&FileUtils::isContainProhibitPath);

    //action3

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });
    EXPECT_EQ(true, opt.checkProhibitPaths(&event));
}

TEST_F(DragDropOperTest, leave)
{
    bool callupdateDragHover = false;
    stub.set_lamda(&DragDropOper::updateDragHover,[&callupdateDragHover](){
        __DBG_STUB_INVOKE__
                callupdateDragHover = true;
    });
    CanvasView view;
    DragDropOper opt(&view);
    opt.m_target = QUrl("temp_url");
    QDragLeaveEvent event;

    opt.leave(&event);
    EXPECT_TRUE(opt.m_target.isEmpty());
    EXPECT_TRUE(callupdateDragHover);
}
class  TestItemModel : public QAbstractItemModel
{
public:
    Q_INVOKABLE virtual QModelIndex index(int row, int column,
                                         const QModelIndex &parent = QModelIndex()) const {
        return QModelIndex();
    }
    Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const {
        return QModelIndex();
    }
    Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const {
        return 1;
    }
    Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const {
        return 1;
    }
    Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
        return QVariant::fromValue(1);
    }

};
using namespace dfmbase;
class TestFileInfo : public FileInfo
{
public:
    using FileInfo::FileInfo;
    void refresh() override {re = true;}
    bool isAttributes(const FileIsType type) const {
        if (type == OptInfoType::kIsHidden)
            return hidden;
        return FileInfo::isAttributes(type);
    }
public:
    bool re = false;
    bool hidden = true;
};
TEST_F(DragDropOperTest, move)
{
    using namespace ddplugin_canvas ;
    typedef QModelIndex(*fun_type)(const QPoint&);
    TestItemModel testItem;
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&testItem](const QPoint&){
        __DBG_STUB_INVOKE__
        QModelIndex res(0, 0, (void *)nullptr,&testItem);
        return res;
    });

    stub.set_lamda(&DragDropOper::updateDragHover,[](){
        __DBG_STUB_INVOKE__
    });

    CanvasProxyModel canvasModel ;
    stub.set_lamda(&CanvasView::model,[&canvasModel](){
        __DBG_STUB_INVOKE__
                return &canvasModel;
    });

    stub.set_lamda(&DragDropOper::checkTargetEnable,[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    stub.set_lamda(&CanvasProxyModel::fileUrl,[&url1](CanvasProxyModel*self, const QModelIndex &index){
        __DBG_STUB_INVOKE__
                return url1;
    });
    stub.set_lamda(&CanvasProxyModel::fileInfo,[](CanvasProxyModel*self,const QModelIndex &index){
        __DBG_STUB_INVOKE__
                FileInfoPointer file(new TestFileInfo(QUrl("file:/home/user/file.txt")));
                return file;
    });
    bool calltryDodge = false;
    stub.set_lamda(&DragDropOper::tryDodge,[&calltryDodge](){
        __DBG_STUB_INVOKE__
                calltryDodge = true;
    });
    CanvasView view;
    DragDropOper opt(&view);
    QMimeData data ;
    QDragMoveEvent event (QPoint(1,1),Qt::CopyAction,&data,Qt::LeftButton,Qt::ShiftModifier);
    ViewHookInterface interface;
    view.d->hookIfs = &interface;

    view.model()->d->fileList.push_back(url1);
    view.model()->d->fileList.push_back(url2);

    EXPECT_TRUE(opt.move(&event));
    EXPECT_FALSE(event.m_accept);
    EXPECT_TRUE(calltryDodge);
}

TEST_F(DragDropOperTest, drop)
{
    stub.set_lamda(&DragDropOper::updatePrepareDodgeValue,[](){
        __DBG_STUB_INVOKE__
    });
    typedef QModelIndex(*fun_type)(const QPoint&);
    TestItemModel testItem;
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&testItem](const QPoint&){
        __DBG_STUB_INVOKE__
        QModelIndex res(0, 0, (void *)nullptr, &testItem);
        return res;
    });
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    stub.set_lamda(&CanvasProxyModel::fileUrl,[&url1](CanvasProxyModel*self, const QModelIndex &index){
        __DBG_STUB_INVOKE__
                return url1;
    });
    stub.set_lamda(&DragDropOper::dropMimeData,[](DragDropOper *self, QDropEvent *){
        __DBG_STUB_INVOKE__
                return false;
    });
    CanvasView view;
    DragDropOper opt(&view);
    ViewHookInterface interface;
    view.d->hookIfs = &interface;
    QMimeData data;
    QDropEvent event (QPointF(),Qt::CopyAction,&data , Qt::LeftButton,Qt::ControlModifier);

    EXPECT_TRUE(opt.drop(&event));
    EXPECT_FALSE(event.m_accept);
}


TEST_F(DragDropOperTest, preproccessDropEvent)
{
    QMimeData data;
    QDropEvent event(QPointF(),Qt::ActionMask,&data , Qt::LeftButton,Qt::ControlModifier);
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    const QList<QUrl> &urls{url1,url2};
    const QUrl targetFileUrl("file://target_url");
    CanvasView view;
    DragDropOper opt(&view);

    opt.preproccessDropEvent(&event,urls,targetFileUrl);

    QObject object ;
    stub.set_lamda(&QDropEvent::source,[&object](){
        __DBG_STUB_INVOKE__
                return &object;
    });
    EXPECT_EQ(event.dropAction(),Qt::MoveAction);
    stub.set_lamda(&ddplugin_canvas::isCtrlPressed,[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    opt.preproccessDropEvent(&event,urls,targetFileUrl);
    EXPECT_EQ(event.dropAction(),Qt::CopyAction);
}

TEST_F(DragDropOperTest, selectItems)
{
    CanvasProxyModel *md = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *sm = new CanvasSelectionModel(md,nullptr);
    stub.set_lamda(&CanvasView::model, [&md]() -> CanvasProxyModel* {
        __DBG_STUB_INVOKE__
        return md;
    });
    stub.set_lamda(&CanvasView::selectionModel, [&sm]() -> CanvasSelectionModel* {
        __DBG_STUB_INVOKE__
        return sm;
    });

    CanvasView *view1 = new CanvasView();
    QSharedPointer<CanvasView> ptr1(view1);
    ptr1->d->state.view = view1;
    ptr1->d->screenNum = 1;

    stub.set_lamda(&CanvasManager::views,[&ptr1]()->QList<QSharedPointer<CanvasView>>{
        __DBG_STUB_INVOKE__
        QList<QSharedPointer<CanvasView>> res;
        res.push_back(ptr1);

        return res;
    });
    stub.set_lamda(&CanvasGrid::point,[](CanvasGrid *self, const QString &item, QPair<int, QPoint> &pos){
        __DBG_STUB_INVOKE__
        if(item == "file://temp_url1"){
            pos.first = 1;
            pos.second = QPoint(1,1);
        }else{
            pos.first = 1;
            pos.second = QPoint(2,2);
        }
        return true;
    });

    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    const QList<QUrl> &urls{url1,url2};
    CanvasView view;
    DragDropOper opt(&view);
    QHash<QString, QPoint> hash;
    hash.insert("file://temp_url1",QPoint(1,1));
    hash.insert("file://temp_url2",QPoint(2,2));
    GridIns->d->itemPos.insert(1,hash);

    opt.selectItems(urls);

    EXPECT_EQ(ptr1->d->operState().current(),QModelIndex());
    EXPECT_EQ(ptr1->d->operState().contBegin,QPersistentModelIndex());

    delete md;
    delete sm;
}

TEST_F(DragDropOperTest, dropFilter)
{
    typedef QModelIndex(*fun_type)(const QPoint&);
    TestItemModel testItem ;
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&testItem](const QPoint&){
        __DBG_STUB_INVOKE__
        QModelIndex res(0, 0, (void *)nullptr, &testItem);
        return res;
    });
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");

    stub.set_lamda(&CanvasProxyModel::fileUrl,[&url1](CanvasProxyModel*self, const QModelIndex &index){
        __DBG_STUB_INVOKE__
                return url1;
    });
    QMimeData data ;
    QDropEvent event(QPointF(),Qt::ActionMask,&data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView view;
    DragDropOper opt(&view);
    ViewHookInterface interface;
    view.d->hookIfs = &interface ;
    EXPECT_FALSE(opt.dropFilter(&event));
}

TEST_F(DragDropOperTest, dropBetweenView)
{
    using namespace ddplugin_canvas ;
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    QList<QUrl> urls{url1};
    QMimeData data ;
    data.setUrls(urls);
    QDropEvent event(QPointF(),Qt::ActionMask,&data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView view;
    view.d->state.view= &view;
    DragDropOper opt(&view);
    EXPECT_FALSE(opt.dropBetweenView(&event));
    stub.set_lamda(&QDropEvent::source,[&view](){
        __DBG_STUB_INVOKE__
                return &view;
    });
    stub.set_lamda(&CanvasManager::update,[](){
        __DBG_STUB_INVOKE__
    });
    TestItemModel testI;
    QModelIndex index(0,0, (void *)nullptr,&testI);
    typedef QModelIndex(*fun_type)(const QPoint&);
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&index](const QPoint& point){
        __DBG_STUB_INVOKE__
        return index;
    });

    CanvasProxyModel md(nullptr);
    CanvasSelectionModel sm(&md,nullptr);
    stub.set_lamda(&CanvasView::selectionModel, [&sm]() -> CanvasSelectionModel* {
        __DBG_STUB_INVOKE__
        return &sm;
    });

    QModelIndex index1;
    stub.set_lamda(&CanvasSelectionModel::selectedIndexesCache,[&index1](){
        __DBG_STUB_INVOKE__
        QModelIndexList res;
        res.push_back(index1);
        return res;
    });
    EXPECT_FALSE(opt.dropBetweenView(&event));

    stub.set_lamda(&CanvasGrid::point,[](CanvasGrid *self, const QString &item, QPair<int, QPoint> &pos){
        __DBG_STUB_INVOKE__
        if(item == "file://temp_url1"){
            pos.first = 1;
            pos.second = QPoint(1,1);
        }else{
            pos.first = 1;
            pos.second = QPoint(2,2);
        }
        return true;
    });
    stub.set_lamda(&CanvasProxyModel::fileUrl,[&url1](CanvasProxyModel*self, const QModelIndex &index){
        __DBG_STUB_INVOKE__
                return url1;
    });
    stub.set_lamda(&CanvasGrid::move,[](CanvasGrid *self, int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items){
        __DBG_STUB_INVOKE__
                return true;
    });
    CanvasProxyModel mdModel(nullptr);
    stub.set_lamda(&CanvasView::model, [&mdModel]() -> CanvasProxyModel* {
        __DBG_STUB_INVOKE__
        return &mdModel;
    });


    QModelIndex indexModel;
    typedef QModelIndex(*fun_type1)(const QUrl &url, int column);
    stub.set_lamda((fun_type1)(QModelIndex(CanvasProxyModel::*)(const QUrl&,int)const)(&CanvasProxyModel::index),[&indexModel](const QUrl &url, int column ){
        __DBG_STUB_INVOKE__
                return indexModel;
    });

    index1 = index;

    EXPECT_TRUE(opt.dropBetweenView(&event));
    EXPECT_EQ(event.dropAction(),Qt::MoveAction);
    EXPECT_EQ(view.d->operState().contBegin,QPersistentModelIndex(indexModel));
    EXPECT_EQ(view.d->operState().current(),indexModel);

    urls.push_back(url2);
    EXPECT_TRUE(opt.dropBetweenView(&event));
    EXPECT_EQ(event.dropAction(),Qt::MoveAction);
}

TEST_F(DragDropOperTest, dropDirectSaveMode)
{
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    typedef QModelIndex(*fun_type)(const QPoint&);
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[](const QPoint&){
        __DBG_STUB_INVOKE__
        TestItemModel testI;
        QModelIndex res(0, 0, (void *)nullptr, &testI);
        return res;
    });
    TestFileInfo *filei = new TestFileInfo(QUrl("file:/home/user/file.txt"));
    stub.set_lamda(&CanvasProxyModel::fileInfo,[&filei](CanvasProxyModel*self,const QModelIndex &index){
        __DBG_STUB_INVOKE__
                FileInfoPointer file(filei);
                return file;
    });
    typedef QUrl(*fun_type1)(const FileInfo::FileUrlInfoType);
    stub.set_lamda((fun_type1)(&FileInfo::urlOf),[url1](){
        __DBG_STUB_INVOKE__
                return url1;
    });

    QMimeData *data =new QMimeData;
    data->setProperty("IsDirectSaveMode",true);
    QDropEvent *event = new QDropEvent(QPointF(),Qt::ActionMask,data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView view;
    DragDropOper opt(&view);

    EXPECT_TRUE(opt.dropDirectSaveMode(event));
    EXPECT_EQ(event->mimeData()->property("DirectSaveUrl"),url1);
    EXPECT_TRUE(event->m_accept);

    delete data;
    delete event;
}

TEST_F(DragDropOperTest, dropMimeData)
{
    CanvasProxyModel *md = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *sm = new CanvasSelectionModel(md,nullptr);
    stub.set_lamda(&CanvasView::model, [md]() -> CanvasProxyModel* {
        __DBG_STUB_INVOKE__
        return md;
    });
    TestItemModel testI;
    QModelIndex index(1, 1, (void *)nullptr,&testI);
    typedef QModelIndex(*fun_type)(const QPoint&);
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&index](const QPoint&){
        __DBG_STUB_INVOKE__
        return index;
    });

    QMimeData *data =new QMimeData;
    QDropEvent *event = new QDropEvent(QPointF(),Qt::ActionMask,data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView view;
    DragDropOper opt(&view);

    EXPECT_FALSE(opt.dropMimeData(event));

    typedef Qt::ItemFlags(*fun_type1)(const QModelIndex&);
    stub.set_lamda((fun_type1)(&QAbstractProxyModel::flags),[](const QModelIndex&){
        __DBG_STUB_INVOKE__
                return Qt::ItemIsDropEnabled;
    });
    typedef bool(*fun_type2)(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    stub.set_lamda((fun_type2)(&CanvasProxyModel::dropMimeData),
                   [](const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent){
        __DBG_STUB_INVOKE__
                return true;
    });

    EXPECT_TRUE(opt.dropMimeData(event));
    EXPECT_TRUE(event->m_accept);
    EXPECT_EQ(event->default_action,event->drop_action);

    delete data;
    delete event;
    delete sm;
    delete md;
}

TEST_F(DragDropOperTest, handleMoveMimeData)
{
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    QMimeData *data =new QMimeData;
    QDropEvent *event = new QDropEvent(QPointF(),Qt::ActionMask,data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView view;
    DragDropOper opt(&view);
    bool on = true;
    stub.set_lamda(&Dtk::Gui::DFileDragClient::checkMimeData,[&on](const QMimeData *data){
        __DBG_STUB_INVOKE__
                return on;
    });
    opt.handleMoveMimeData(event,url1);
    EXPECT_EQ(event->default_action,event->drop_action);
    on = false;
    opt.handleMoveMimeData(event,url1);
    EXPECT_TRUE(event->m_accept);

    delete data;
    delete event;
}



TEST_F(DragDropOperTest, dropClientDownload)
{
    stub.set_lamda(&DFileDragClient::checkMimeData, []() -> bool {
        return true;
    });
    QUrl url1 = QUrl::fromLocalFile("file://temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file://temp_url2");
    QList<QUrl> qurls{url1,url2};
    QMimeData *data =new QMimeData;
    data->setUrls(qurls);
    QDropEvent *event = new QDropEvent(QPointF(),Qt::ActionMask,data , Qt::LeftButton,Qt::ControlModifier);
    CanvasView *view = new CanvasView ;
    DragDropOper *opt  = new DragDropOper(view) ;

    EXPECT_TRUE(opt->dropClientDownload(event));
    delete event;
    delete data;

}

TEST_F(DragDropOperTest, dragDropOperTest)
{

    QMimeData data;

    CanvasView view;
    DragDropOper opt(&view);
    QEvent qevent(QEvent::None);

    bool callupdatePrepareDodgeValue = false;
    stub.set_lamda(&DodgeOper::updatePrepareDodgeValue,[&callupdatePrepareDodgeValue](){
        __DBG_STUB_INVOKE__
                callupdatePrepareDodgeValue = true;
    });

    opt.updatePrepareDodgeValue(&qevent);
    EXPECT_TRUE(callupdatePrepareDodgeValue);

    QDragMoveEvent dmoveEvent(QPoint(1,1),Qt::CopyAction,&data,Qt::LeftButton,Qt::ShiftModifier);
    bool calltryDodge = false;
    stub.set_lamda(&DodgeOper::tryDodge,[&calltryDodge](DodgeOper *self,QDragMoveEvent *event){
        __DBG_STUB_INVOKE__
                calltryDodge = true;
    });
    opt.tryDodge(&dmoveEvent);
    EXPECT_TRUE(calltryDodge);

    bool callupdate = false;
    typedef void(*fun_type)(const QModelIndex&);
    stub.set_lamda((fun_type)(void(QAbstractItemView::*)(const QModelIndex&))(&QAbstractItemView::update),
                   [&callupdate]( const QModelIndex &){
        __DBG_STUB_INVOKE__
              callupdate = true;
    });
    typedef QModelIndex(*fun_type1)(const QPoint&);
    TestItemModel testi;
    stub.set_lamda((fun_type1)(&CanvasView::indexAt),[&testi](const QPoint&){
        __DBG_STUB_INVOKE__
        QModelIndex res(0, 0, (void *)nullptr, &testi);
        return res;
    });

    opt.updateDragHover(QPoint(1,1));
    EXPECT_TRUE(callupdate);

    bool callstopDelayDodge = false;
    stub.set_lamda(&DodgeOper::stopDelayDodge,[&callstopDelayDodge](){
        __DBG_STUB_INVOKE__
              callstopDelayDodge = true;
    });
    opt.stopDelayDodge();
    EXPECT_TRUE(callstopDelayDodge);

    opt.updateDFMMimeData(&dmoveEvent);
    EXPECT_TRUE(opt.checkTargetEnable(QUrl("file:/temp_url")));
}
