// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionview_p.h"
#include "view/collectionview.h"
#include "delegate/collectionitemdelegate_p.h"
#include "interface/canvasmanagershell.h"
#include "mode/normalized/type/typeclassifier.h"
#include "models/collectionmodel_p.h"
#include "interface/fileinfomodelshell.h"

#include <dfm-base/dfm_global_defines.h>
#include "stubext.h"

#include <gtest/gtest.h>
#include <DApplication>

#include <QMenu>
#include <QItemSelection>
#include <QItemSelectionRange>
#include <QMimeData>
#include <QScrollBar>

using namespace testing;
using namespace dfmbase;
using namespace ddplugin_organizer;

TEST(CollectionViewPrivate, helpAction) {

    stub_ext::StubExt stub;
    using namespace Dtk::Widget;

    typedef void (*fptr)(DApplication*);
    fptr utHandleHelp = (fptr)((void(DApplication::*)())&DApplication::handleHelpAction);

    bool isCall = false;
    stub.set_lamda(utHandleHelp, [&] () {
        isCall = true;
    });

    EXPECT_FALSE(isCall);

    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);
    view.d->helpAction();
    EXPECT_TRUE(isCall);
}

TEST(CollectionViewPrivate, onIconSizeChanged) {

    stub_ext::StubExt stub;
    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);

    int setLevel = -1;
    stub.set_lamda(ADDR(CollectionItemDelegate, setIconLevel), [&] (CollectionItemDelegate *obj, int lv) {
        Q_UNUSED(obj)

        setLevel = lv;
        return lv;
    });

    view.d->onIconSizeChanged(1);

    EXPECT_EQ(setLevel, 1);
}

TEST(CollectionView, setCanvasManagerShell) {

    stub_ext::StubExt stub;
    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);

    CanvasManagerShell *sh = nullptr;

    EXPECT_EQ(view.d->canvasManagerShell, nullptr);

    view.setCanvasManagerShell(sh);
    EXPECT_EQ(view.d->canvasManagerShell, nullptr);

    int setLevel = -1;
    bool isCallSetIconLevel = false;
    stub.set_lamda(ADDR(CollectionItemDelegate, setIconLevel), [&] (CollectionItemDelegate *obj, int lv) {
        Q_UNUSED(obj)

        isCallSetIconLevel = true;
        setLevel = lv;
        return lv;
    });

    bool isCallIconLevel = false;
    stub.set_lamda(ADDR(CanvasManagerShell, iconLevel), [&] () {
        isCallIconLevel = true;
        return 3;
    });

    sh = new CanvasManagerShell;
    view.setCanvasManagerShell(sh);
    EXPECT_TRUE(isCallIconLevel);
    EXPECT_TRUE(isCallSetIconLevel);

    isCallIconLevel = false;
    isCallSetIconLevel = false;
    view.setCanvasManagerShell(nullptr);
    EXPECT_FALSE(isCallIconLevel);
    EXPECT_FALSE(isCallSetIconLevel);

    delete sh;
    sh = nullptr;
}

TEST(CollectionView, scrollContentsBy)
{
    stub_ext::StubExt stub;
    CollectionView view("dd", nullptr);
    int dx = -1;
    int dy = -1;
    QWidget *port = nullptr;
    stub.set_lamda((void (QWidget::*)(int, int))&QWidget::scroll, [&](QWidget *self, int x, int y) {
        port = self;
        dx = x;
        dy = y;
        return ;
    });

    view.scrollContentsBy(0, 100);
    EXPECT_EQ(port, view.viewport());
    EXPECT_EQ(dx, 0);
    EXPECT_EQ(dy, 100);
}

class TestProvider : public CollectionDataProvider
{
public:
    TestProvider() : CollectionDataProvider(nullptr)
    {

    }
protected:
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) {return "";}
    QString append(const QUrl &) {return "";}
    QString prepend(const QUrl &){return "";}
    void insert(const QUrl &, const QString &, const int) {}
    QString remove(const QUrl &) {return "";}
    QString change(const QUrl &) {return "";}
};

TEST(CollectionView, sort)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    bool callLess = false;
    stub.set_lamda(&CollectionView::lessThan, [&callLess]() {
        callLess = true;
        return false;
    });

    ASSERT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    ASSERT_EQ(view.d->sortOrder, Qt::DescendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortOrder, Qt::AscendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortOrder, Qt::DescendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(view.d->sortOrder, Qt::AscendingOrder);
}

TEST(CollectionView, keyPressEvent_space)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    bool preview = false;
    stub.set_lamda(&CollectionViewPrivate::previewFiles, [&preview](){
        preview = true;;
    });

    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier,"", true);
        view.keyPressEvent(&key);
        EXPECT_FALSE(preview);
    }

    {
        preview = false;
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier,"", false);
        view.keyPressEvent(&key);
        EXPECT_TRUE(preview);
    }
}

TEST(CollectionView, keyPressEvent_I)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    bool show = false;
    stub.set_lamda(&CollectionViewPrivate::showFilesProperty, [&show](){
        show = true;;
    });

    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier,"", false);
        view.keyPressEvent(&key);
        EXPECT_TRUE(show);
    }
}

TEST(CollectionView, dataProvider)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    EXPECT_EQ(&test, view.dataProvider());
}

TEST(CollectionView, keyboardSearch)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    bool find = false;
    stub.set_lamda(&CollectionViewPrivate::findIndex, [&find](){
        find = true;
        return QModelIndex();
    });

    ASSERT_NE(view.d->searchTimer, nullptr);
    view.d->searchKeys.clear();

    view.keyboardSearch("");
    EXPECT_TRUE(view.d->searchKeys.isEmpty());
    EXPECT_FALSE(find);
    EXPECT_FALSE(view.d->searchTimer->isActive());

    view.d->searchTimer->stop();
    view.d->searchKeys.clear();
    find = false;

    view.keyboardSearch("1");
    QTimer wait;
    QEventLoop loop;
    loop.connect(&wait, &QTimer::timeout, &loop, &QEventLoop::quit);
    wait.start(220);
    EXPECT_EQ(view.d->searchKeys, QString("1"));
    EXPECT_TRUE(find);
    ASSERT_TRUE(view.d->searchTimer->isActive());

    loop.exec();
    EXPECT_TRUE(view.d->searchKeys.isEmpty());
}

TEST(CollectionView, isIndexHidden)
{
    stub_ext::StubExt stub;
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///etc");

    stub.set_lamda(VADDR(TestProvider, contains), [url2](CollectionDataProvider *self, const QString &key, const QUrl &url) {
        return key == "dd" && url == url2;
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    view.setModel(&model);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    EXPECT_TRUE(view.isIndexHidden(idx1));
    EXPECT_FALSE(view.isIndexHidden(idx2));
}

TEST(CollectionView, selectedIndexes)
{
    stub_ext::StubExt stub;
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///etc");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url2};
    });

    stub.set_lamda(VADDR(TestProvider, contains), [url2](CollectionDataProvider *self, const QString &key, const QUrl &url) {
        return key == "dd" && url == url2;
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    stub.set_lamda(VADDR(QAbstractProxyModel,flags), [](){
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    });

    view.setModel(&model);

    view.selectionModel()->select(QItemSelection(idx1, idx2), QItemSelectionModel::ClearAndSelect);
    ASSERT_EQ(view.selectionModel()->selectedIndexes().size(), 2);

    auto idxs = view.selectedIndexes();
    ASSERT_EQ(idxs.size(), 1);
    ASSERT_EQ(idxs.first(), idx2);
}

TEST(CollectionView, selectRect)
{
    stub_ext::StubExt stub;
    TestProvider test;
    CollectionView view("dd", &test);
    CollectionModel model;
    view.setModel(&model);

    stub.set_lamda(&CollectionViewPrivate::selection,[](){
        return QItemSelection();
    });

    QItemSelectionModel::SelectionFlags flag;
    stub.set_lamda((void (*)(QItemSelectionModel *, const QItemSelection &, QItemSelectionModel::SelectionFlags))
                   ((void (QItemSelectionModel::*)(const QItemSelection &, QItemSelectionModel::SelectionFlags))&QItemSelectionModel::select),
                   [&flag](QItemSelectionModel *self, const QItemSelection &, QItemSelectionModel::SelectionFlags cmd){
        flag = cmd;
    });

    Qt::KeyboardModifiers key = Qt::NoModifier;
    stub.set_lamda(&QGuiApplication::keyboardModifiers,[&key](){
        return key;
    });

    QRect r(QPoint(0,0), view.size());
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::ClearAndSelect);

    key = Qt::ControlModifier;
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::ToggleCurrent);

    key = Qt::ShiftModifier;
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::SelectCurrent);
}

TEST(CollectionView, findIndex)
{
    stub_ext::StubExt stub;
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///un");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url1, url2};
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    view.setModel(&model);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    stub.set_lamda(VADDR(CollectionModel, data), [idx1,idx2](CollectionModel *, const QModelIndex &index, int role) {
        QVariant var;
        if (role ==  Global::ItemRoles::kItemFilePinyinNameRole) {
           if (index == idx1)
               var = QVariant::fromValue(QString("usr"));
           else if (index == idx2)
               var = QVariant::fromValue(QString("un"));
        }
        return var;
    });

    EXPECT_EQ(view.d->findIndex("u", true, QModelIndex(), false, false), idx1);
    EXPECT_EQ(view.d->findIndex("u", true, idx1, false, true), idx2);
}

TEST(CollectionView, dragEnterEvent) {
    stub_ext::StubExt stub;
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///un");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url1, url2};
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    view.setModel(&model);
    bool isCallfileUrl { false };
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
                       return {};
                   });

    // action1
    stub.set_lamda(&CollectionViewPrivate::checkProhibitPaths, []() -> bool{
        return true;
    });

    stub.set_lamda(&CollectionView::model, [&]() -> CollectionModel* {
        return &model;
    });

    stub.set_lamda(&CollectionModel::fileUrl, [&]() -> QUrl {
        isCallfileUrl = true;
        return QUrl();
    });

    stub.set_lamda(&CollectionModel::rootIndex, []() -> QModelIndex {
        return QModelIndex();
    });

    view.dragEnterEvent(&event);
    EXPECT_TRUE(!isCallfileUrl);

    // action2
    isCallfileUrl = false;
    stub.reset(&CollectionViewPrivate::checkProhibitPaths);
    stub.set_lamda(&CollectionViewPrivate::checkProhibitPaths, []() -> bool{
        return false;
    });

    stub.set_lamda(&CollectionViewPrivate::checkClientMimeData, []() -> bool{
        return true;
    });

    bool isCallCheckXdndDirectSave { false };
    stub.set_lamda(&CollectionViewPrivate::checkXdndDirectSave, [&]() -> bool{
        isCallCheckXdndDirectSave = true;
        return false;
    });

    view.dragEnterEvent(&event);
    EXPECT_TRUE(isCallfileUrl);
    EXPECT_TRUE(!isCallCheckXdndDirectSave);


    // action3
    isCallfileUrl = false;
    stub.reset(&CollectionViewPrivate::checkClientMimeData);
    stub.reset(&CollectionViewPrivate::checkProhibitPaths);
    stub.set_lamda(&CollectionViewPrivate::checkProhibitPaths, [&]() -> bool{
        return false;
    });
    stub.set_lamda(&CollectionViewPrivate::checkClientMimeData, [&]() -> bool{
        return false;
    });

    stub.set_lamda(&CollectionViewPrivate::checkXdndDirectSave, [&]() -> bool{
        return false;
    });

    bool isCall { false };
    typedef void (*func)(CollectionViewPrivate*,QDropEvent *event, const QUrl &targetFileUrl);
    auto testFunc = (func)(&CollectionViewPrivate::preproccessDropEvent);
    stub.set_lamda(testFunc, [&]() {
        isCall = true;
        return;
    });

    view.dragEnterEvent(&event);
    EXPECT_TRUE(isCallfileUrl);
    EXPECT_TRUE(isCall);
}

TEST(CollectionViewPrivate, updateVerticalBarRange)
{
    TestProvider test;
    CollectionView view("dd", &test);
    view.d->updateVerticalBarRange();

    EXPECT_EQ(view.verticalScrollBar()->pageStep(), view.viewport()->height());
}

TEST(CollectionViewPrivate, pointAndPos)
{
    CollectionView view("", nullptr);
    CollectionViewPrivate *cvp = view.d.get();
    cvp->rowCount = 2;
    cvp->columnCount = 3;
    cvp->cellHeight = 40;
    cvp->cellWidth = 50;
    cvp->viewMargins = QMargins(1,1,1,1);
    stub_ext::StubExt stub;
    stub.set_lamda(&CollectionViewPrivate::initUI, [](){
        return;
    });

    stub.set_lamda(&CollectionViewPrivate::initConnect, [](){
        return;
    });

    {
        EXPECT_EQ(cvp->pointToPos(QPoint(1, 1)), QPoint(0,0));
        EXPECT_EQ(cvp->pointToPos(QPoint(51, 1)), QPoint(1,0));
        EXPECT_EQ(cvp->pointToPos(QPoint(101, 1)), QPoint(2,0));
        EXPECT_EQ(cvp->pointToPos(QPoint(1, 41)), QPoint(0,1));
        EXPECT_EQ(cvp->pointToPos(QPoint(1, 81)), QPoint(0,2));
        EXPECT_EQ(cvp->pointToPos(QPoint(51, 41)), QPoint(1,1));
    }

    {
        EXPECT_EQ(cvp->posToPoint(QPoint(0, 0)), QPoint(1,1));
        EXPECT_EQ(cvp->posToPoint(QPoint(1, 0)), QPoint(51,1));
        EXPECT_EQ(cvp->posToPoint(QPoint(2, 0)), QPoint(101,1));
        EXPECT_EQ(cvp->posToPoint(QPoint(0, 1)), QPoint(1,41));
        EXPECT_EQ(cvp->posToPoint(QPoint(1, 1)), QPoint(51,41));
    }

    {
        EXPECT_EQ(cvp->posToNode(QPoint(0, 0)), 0);
        EXPECT_EQ(cvp->posToNode(QPoint(1, 0)), 1);
        EXPECT_EQ(cvp->posToNode(QPoint(2, 0)), 2);
        EXPECT_EQ(cvp->posToNode(QPoint(0, 1)), 3);
        EXPECT_EQ(cvp->posToNode(QPoint(1, 1)), 4);
        EXPECT_EQ(cvp->posToNode(QPoint(2, 1)), 5);
    }

    {
        EXPECT_EQ(cvp->nodeToPos(0), QPoint(0, 0));
        EXPECT_EQ(cvp->nodeToPos(1), QPoint(1, 0));
        EXPECT_EQ(cvp->nodeToPos(2), QPoint(2, 0));
        EXPECT_EQ(cvp->nodeToPos(3), QPoint(0, 1));
        EXPECT_EQ(cvp->nodeToPos(4), QPoint(1, 1));
        EXPECT_EQ(cvp->nodeToPos(5), QPoint(2, 1));
    }
}

namespace  {

class TestSourceModel : public QAbstractItemModel
{
public:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override{
        return createIndex(row, column, nullptr);
    }
    QModelIndex parent(const QModelIndex &child) const override {
        return QModelIndex();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        return 2;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
        return QVariant();
    }
    Qt::ItemFlags flags(const QModelIndex &index) const {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

};

class TestFileClassifier : public FileClassifier
{
public:
    Classifier mode() const {return kName;}
    ModelDataHandler *dataHandler() const {return nullptr;}
    QStringList classes() const {return ids.keys();}
    QString classify(const QUrl &url) const {return ids.key(url.fileName().left(3));}
    QString className(const QString &key) const {return ids.value(key);}
    QMap<QString, QString> ids;
};

class TestCollectionView : public Test
{
public:
    virtual void SetUp() override {
        view = new CollectionView("1", &classifier);
        classifier.ids.insert("1", "one");
        classifier.ids.insert("2", "two");
        shell.model = &srcModel;
        model = new CollectionModel;
        model->setModelShell(&shell);

        model->d->fileList.append(one1);
        model->d->fileMap.insert(one1, FileInfoPointer());
        model->d->fileList.append(one2);
        model->d->fileMap.insert(one2, FileInfoPointer());
        model->d->fileList.append(two1);
        model->d->fileMap.insert(two1, FileInfoPointer());
        model->d->fileList.append(test);
        model->d->fileMap.insert(test, FileInfoPointer());

        dlg = new CollectionItemDelegate(view);

        dp.reset(new CollectionBaseData);
        dp->name = "one";
        dp->key = "1";
        dp->items.append(one1);
        dp->items.append(one2);
        classifier.collections.insert("1", dp);

        dp2.reset(new CollectionBaseData);
        dp2->name = "two";
        dp2->key = "2";
        dp2->items.append(two1);
        classifier.collections.insert("2", dp2);
        view->setModel(model);
        view->d->rowCount = 2;
        view->d->columnCount = 3;
        view->d->cellWidth = 60;
        view->d->cellHeight = 50;
        view->d->cellMargins = QMargins(2,2,2,2);
        view->d->viewMargins = QMargins(1,1,1,1);
    }

    virtual void TearDown() override {
        stub.clear();
        delete view;
        delete model;
    }

    stub_ext::StubExt stub;
    CollectionView *view;
    CollectionModel *model;
    TestSourceModel srcModel;
    FileInfoModelShell shell;
    CollectionItemDelegate *dlg;
    TestFileClassifier classifier;

    QUrl one1 = QUrl::fromLocalFile("/tmp/one1");
    QUrl one2 = QUrl::fromLocalFile("/tmp/one2");
    QUrl two1 = QUrl::fromLocalFile("/tmp/two1");
    QUrl test = QUrl::fromLocalFile("/tmp/test");

    CollectionBaseDataPtr dp;
    CollectionBaseDataPtr dp2;
};
}

TEST_F(TestCollectionView, updateRegionView)
{
    view->setGeometry(0,0, 300, 200);
    view->setIconSize(QSize(48, 48));
    stub.set_lamda(&CollectionItemDelegate::updateItemSizeHint, [](CollectionItemDelegate *self){
        self->d->itemSizeHint = QSize(48,48);
    });

    view->d->updateRegionView();
    EXPECT_EQ(view->d->viewMargins, QMargins(6,2,6,2));
    EXPECT_EQ(view->d->columnCount, 6);
    EXPECT_EQ(view->d->cellWidth, 48);
    EXPECT_EQ(view->d->cellHeight, 50);
    EXPECT_EQ(view->d->rowCount, 4);
    EXPECT_EQ(view->d->cellMargins, QMargins(0,1,0,1));
}

TEST_F(TestCollectionView, visualRect)
{
    EXPECT_EQ(view->visualRect(model->index(0,0)), QRect(1,1,60,50));
    EXPECT_EQ(view->visualRect(model->index(1,0)), QRect(61,1,60,50));
}

TEST_F(TestCollectionView, itemRect)
{
    EXPECT_EQ(view->d->itemRect(model->index(0,0)), QRect(3,3,56,46));
    EXPECT_EQ(view->d->itemRect(model->index(1,0)), QRect(63,3,56,46));
}

TEST_F(TestCollectionView, itemPaintGeomertys)
{
    EXPECT_EQ(view->d->itemPaintGeomertys(model->index(0,0)).size(), 3);
}

TEST_F(TestCollectionView, polymerizePixmap)
{
    EXPECT_TRUE(view->d->polymerizePixmap({}).isNull());
    auto pix = view->d->polymerizePixmap({model->index(0,0)});
    EXPECT_EQ(pix.size(), QSize(188, 188) * view->devicePixelRatioF());
}

TEST_F(TestCollectionView, indexAt)
{
    view->d->columnCount = 1;
    view->d->rowCount = 3;
    stub_ext::StubExt stub;
    QModelIndex open;
    stub.set_lamda(&CollectionView::isPersistentEditorOpen, [&open,this](QAbstractItemView *, const QModelIndex &index){
        return open == index;
    });

    QWidget editor;
    editor.setGeometry(view->d->visualRect(QPoint(0, 0)).marginsAdded(QMargins(0, 0, 0, 20)));
    stub.set_lamda(&CollectionView::indexWidget, [&editor](){
        return &editor;
    });

    QModelIndex expend;
    stub.set_lamda(&CollectionItemDelegate::mayExpand, [&expend](CollectionItemDelegate *, QModelIndex *who){
        *who = expend;
        return expend.isValid();
    });

    stub.set_lamda(&CollectionViewPrivate::itemPaintGeomertys, [&expend,this](CollectionViewPrivate *, const QModelIndex &index){
        QList<QRect> ret;
        if (model->index(0, 0) == index) {
            if (expend != index)
                ret.append(view->d->visualRect(QPoint(0, 0)));
            else
                ret.append(view->d->visualRect(QPoint(0, 0)).marginsAdded(QMargins(0, 0, 0, 20)));
        } else if (model->index(1, 0) == index) {
            ret.append(view->d->visualRect(QPoint(0, 1)));
        }

        return ret;
    });

    // with editor
    {
        expend = QModelIndex();
        open = model->index(0, 0);
        view->setCurrentIndex(open);
        // on view margins
        EXPECT_FALSE(view->indexAt(QPoint(0, 1)).isValid());

        // on item
        EXPECT_EQ(view->indexAt(QPoint(1, 1)), model->index(0, 0));

        // on expended area of editor.
        EXPECT_EQ(view->indexAt(QPoint(10, 70)), model->index(0, 0));

        // out of expended area of editor.
        EXPECT_EQ(view->indexAt(QPoint(10, 71)), model->index(1, 0));
    }

    // with expended item
    {
        open = QModelIndex();
        expend = model->index(0, 0);

        // on item
        view->setCurrentIndex(model->index(0, 0));
        EXPECT_EQ(view->indexAt(QPoint(1, 1)), model->index(0, 0));

        view->setCurrentIndex(QModelIndex());
        EXPECT_EQ(view->indexAt(QPoint(1, 1)), model->index(0, 0));

        // on expend area
        EXPECT_EQ(view->indexAt(QPoint(10, 70)), model->index(0, 0));

        // out of expend area
        EXPECT_EQ(view->indexAt(QPoint(10, 71)), model->index(1, 0));
    }

    {
        expend = QModelIndex();
        view->setCurrentIndex(QModelIndex());

        EXPECT_FALSE(view->indexAt(QPoint(1, 101)).isValid());
        EXPECT_EQ(view->indexAt(QPoint(1, 50)), model->index(0, 0));
        EXPECT_EQ(view->indexAt(QPoint(1, 100)), model->index(1, 0));
    }
}

TEST_F(TestCollectionView, selection)
{
    EXPECT_TRUE(view->d->selection(QRect(0,0,1,1)).isEmpty());

    auto ret = view->d->selection(QRect(11,11,10,10)).indexes();
    ASSERT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.first(), model->index(0,0));

    ret = view->d->selection(QRect(71,11,10,10)).indexes();
    ASSERT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.first(), model->index(1,0));

    ret = view->d->selection(QRect(11,11,81,10)).indexes();
    ASSERT_EQ(ret.size(), 2);
    EXPECT_EQ(ret.first(), model->index(0,0));
    EXPECT_EQ(ret.last(), model->index(1,0));

    EXPECT_TRUE(view->d->selection(QRect(111,1,10,10)).isEmpty());
    EXPECT_TRUE(view->d->selection(QRect(1,41,10,10)).isEmpty());
}
