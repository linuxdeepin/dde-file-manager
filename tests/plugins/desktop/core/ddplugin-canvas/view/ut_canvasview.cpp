// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/canvasitemdelegate_p.h"
#include "view/canvasview_p.h"
#include "view/operator/viewpainter.h"
#include "model/fileinfomodel_p.h"
#include "model/canvasproxymodel_p.h"
#include "model/canvasselectionmodel.h"
#include "displayconfig.h"
#include "watermask/deepinlicensehelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/syncfileinfo.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QScrollBar>
#include <QWindow>
#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_canvas;

TEST(CanvasView, initUI)
{
    FileInfoModel fmodel;
    fmodel.d->fileProvider->setRoot(QUrl::fromLocalFile("/tmp"));
    CanvasProxyModel model;
    model.setSourceModel(&fmodel);

    CanvasView view;
    EXPECT_EQ(view.selectionMode(), QAbstractItemView::NoSelection);
    EXPECT_EQ(view.selectionBehavior(), QAbstractItemView::SelectItems);

    view.setModel(&model);

    CanvasSelectionModel sel(&model, nullptr);
    view.setSelectionModel(&sel);

    stub_ext::StubExt stub;
    stub.set_lamda(&DisplayConfig::iconLevel, [](){
        return 3;
    });

    stub.set_lamda(&CanvasViewPrivate::isWaterMaskOn, [](){
        return true;
    });

    bool initlicense = false;
    stub.set_lamda(&DeepinLicenseHelper::init, [&initlicense](){
        initlicense = true;
    });

    bool refreshMask = false;
    stub.set_lamda(&WaterMaskFrame::refresh, [&refreshMask](){
        refreshMask = true;
    });

    view.initUI();

    EXPECT_TRUE(view.testAttribute(Qt::WA_TranslucentBackground));
    EXPECT_TRUE(view.testAttribute(Qt::WA_InputMethodEnabled));
    EXPECT_TRUE(view.viewport()->testAttribute(Qt::WA_TranslucentBackground));
    EXPECT_FALSE(view.viewport()->autoFillBackground());

    EXPECT_TRUE(view.acceptDrops());

    ASSERT_NE(view.itemDelegate(), nullptr);
    EXPECT_EQ(view.itemDelegate()->iconLevel(), 3);
    EXPECT_EQ(view.iconSize(), QSize(96, 96));

    EXPECT_EQ(view.rootIndex(), model.rootIndex());
    ASSERT_NE(view.d->waterMask, nullptr);
    EXPECT_TRUE(initlicense);
    EXPECT_TRUE(refreshMask);
    EXPECT_EQ(view.d->waterMask->configFile,
              QString("/usr/share/deepin/dde-desktop-watermask.json"));

}

TEST(CanvasView, verticalAndhorizontalOffset)
{
    CanvasView view;
    QSignalBlocker hb(view.horizontalScrollBar());
    QSignalBlocker vb(view.verticalScrollBar());

    view.horizontalScrollBar()->setRange(0, 30);
    view.horizontalScrollBar()->setValue(20);
    view.verticalScrollBar()->setRange(0, 30);
    view.verticalScrollBar()->setValue(10);

    EXPECT_EQ(view.horizontalOffset(), 20);
    EXPECT_EQ(view.verticalOffset(), 10);
}

TEST(CanvasView, winId)
{
    {
        CanvasView view;
        view.createWinId();
        EXPECT_EQ(view.winId(), view.windowHandle()->winId());
    }

    {
        QWidget wid;
        wid.createWinId();
        CanvasView view(&wid);
        EXPECT_EQ(view.winId(), wid.winId());
    }
}

TEST(CanvasView, focusInEvent)
{
    CanvasView view;
    QFocusEvent e(QEvent::FocusIn);
    view.setAttribute(Qt::WA_InputMethodEnabled, false);
    view.focusInEvent(&e);
    EXPECT_TRUE(view.testAttribute(Qt::WA_InputMethodEnabled));
}

class TestCanvasViewPrivate : public testing::Test
{
public:
    void SetUp() override {
        cvp = view.d.data();
        cvp->gridMargins = QMargins(2, 2, 2, 2);
        cvp->viewMargins = QMargins(5, 5, 5, 5);
        cvp->canvasInfo = CanvasViewPrivate::CanvasInfo(10, 10, 50, 50);
    }
    CanvasView view;
    CanvasViewPrivate *cvp;
};

TEST_F(TestCanvasViewPrivate, inline_foo)
{
    EXPECT_EQ(CanvasViewPrivate::gridMiniMargin, QMargins(2,2,2,2));
    EXPECT_EQ(CanvasViewPrivate::dockReserveSize, QSize(80, 80));
    EXPECT_EQ(cvp->canvasInfo.rowCount, 10);
    EXPECT_EQ(cvp->canvasInfo.columnCount, 10);
    EXPECT_EQ(cvp->canvasInfo.gridWidth, 50);
    EXPECT_EQ(cvp->canvasInfo.gridHeight, 50);

    EXPECT_EQ(cvp->canvasInfo.gridCount(), 100);
    EXPECT_EQ(CanvasViewPrivate::gridMarginsHelper(&view), cvp->gridMargins);
    EXPECT_EQ(CanvasViewPrivate::gridMarginsHelper(nullptr), QMargins(0,0,0,0));

    {
        auto coord = cvp->gridCoordinate(1);
        EXPECT_EQ(coord.x(), 0);
        EXPECT_EQ(coord.y(), 1);
        EXPECT_EQ(cvp->gridIndex(coord.point()), 1);

        coord = cvp->gridCoordinate(2);
        EXPECT_EQ(coord.x(), 0);
        EXPECT_EQ(coord.y(), 2);
        EXPECT_EQ(cvp->gridIndex(coord.point()), 2);

        coord = cvp->gridCoordinate(22);
        EXPECT_EQ(coord.x(), 2);
        EXPECT_EQ(coord.y(), 2);
        EXPECT_EQ(cvp->gridIndex(coord.point()), 22);
    }

    EXPECT_EQ(cvp->overlapPos(), QPoint(9, 9));

    EXPECT_EQ(cvp->gridAt(QPoint(54, 54)), QPoint(0,0));
    EXPECT_EQ(cvp->gridAt(QPoint(55, 54)), QPoint(1,0));
    EXPECT_EQ(cvp->gridAt(QPoint(56, 56)), QPoint(1,1));
    EXPECT_EQ(cvp->gridAt(QPoint(200, 200)), QPoint(3,3));
}

TEST_F(TestCanvasViewPrivate, updateGridSize_invalidArea)
{
    cvp->updateGridSize(QSize(100,100), QMargins(5,5,5,5), QSize(88, 88));
    EXPECT_EQ(cvp->canvasInfo.rowCount, 1);
    EXPECT_EQ(cvp->canvasInfo.columnCount, 1);
    EXPECT_EQ(cvp->canvasInfo.gridWidth, 90);
    EXPECT_EQ(cvp->canvasInfo.gridHeight, 90);

    EXPECT_EQ(cvp->gridMargins, QMargins(1, 1, 1, 1));
    EXPECT_EQ(cvp->viewMargins, QMargins(5,5,5,5));
}

TEST_F(TestCanvasViewPrivate, updateGridSize)
{
    cvp->updateGridSize(QSize(1000, 800), QMargins(5,5,5,5), QSize(90, 90));
    EXPECT_EQ(cvp->canvasInfo.rowCount, 7);
    EXPECT_EQ(cvp->canvasInfo.columnCount, 9);
    EXPECT_EQ(cvp->canvasInfo.gridWidth, 110);
    EXPECT_EQ(cvp->canvasInfo.gridHeight, 112);

    EXPECT_EQ(cvp->gridMargins, QMargins(10, 11, 10, 11));
    EXPECT_EQ(cvp->viewMargins, QMargins(5, 8, 5, 8));
}

TEST_F(TestCanvasViewPrivate, calcMargins)
{
    auto ret = cvp->calcMargins(QSize(90, 90), QSize(100, 90));
    EXPECT_EQ(ret, QMargins(5, 0, 5, 0));

    ret = cvp->calcMargins( QSize(80, 80), QSize(100, 102));
    EXPECT_EQ(ret, QMargins(10, 11, 10, 11));

    ret = cvp->calcMargins(QSize(90, 90), QSize(80, 80));
    EXPECT_EQ(ret, QMargins(0, 0, 0, 0));
}

TEST_F(TestCanvasViewPrivate, visualRect)
{
    auto ret = cvp->visualRect(QPoint(0, 0));
    EXPECT_EQ(ret, QRect(5, 5, 50, 50));

    ret = cvp->visualRect(QPoint(1, 1));
    EXPECT_EQ(ret, QRect(55, 55, 50, 50));

    ret = cvp->visualRect(QPoint(9, 9));
    EXPECT_EQ(ret, QRect(455, 455, 50, 50));
}

TEST_F(TestCanvasViewPrivate, itemRect)
{
    auto ret = cvp->itemRect(QPoint(0, 0));
    EXPECT_EQ(ret, QRect(7, 7, 46, 46));

    ret = cvp->itemRect(QPoint(1, 1));
    EXPECT_EQ(ret, QRect(57, 57, 46, 46));

    ret = cvp->itemRect(QPoint(9, 9));
    EXPECT_EQ(ret, QRect(457, 457, 46, 46));
}

TEST_F(TestCanvasViewPrivate, itemGridpos)
{
    cvp->screenNum = 1;
    stub_ext::StubExt stub;
    QString it;
    bool fakeRet = false;
    QPair<int, QPoint> fake;
    stub.set_lamda(&CanvasGrid::point, [&it, &fakeRet, &fake](CanvasGrid *,
                   const QString &item, QPair<int, QPoint> &pos){
        pos = fake;
        it = item;
        return fakeRet;
    });
    const QString file = "/var/desktop-test";
    stub.set_lamda(&CanvasGrid::overloadItems, [file](CanvasGrid *, int num){
        if (num == 2)
            return QStringList{file};
        return QStringList();
    });

    {
        it = "test";
        QPoint pos(99, 99);
        EXPECT_FALSE(cvp->itemGridpos("", pos));
        EXPECT_FALSE(it.isEmpty());
        EXPECT_EQ(pos, QPoint(99, 99));
    }

    {
        fakeRet = true;
        it.clear();
        fake = {2, QPoint(1,1)};
        QPoint pos(99, 99);
        EXPECT_FALSE(cvp->itemGridpos(file, pos));
        EXPECT_EQ(it, file);
        EXPECT_EQ(pos, QPoint(99, 99));
    }

    {
        fakeRet = true;
        it.clear();
        fake = {1, QPoint(1,1)};
        QPoint pos(99, 99);
        EXPECT_TRUE(cvp->itemGridpos(file, pos));
        EXPECT_EQ(it, file);
        EXPECT_EQ(pos, QPoint(1,1));
    }

    {
        fakeRet = false;
        it.clear();
        QPoint pos(99, 99);
        EXPECT_FALSE(cvp->itemGridpos(file, pos));
        EXPECT_EQ(it, file);
        EXPECT_EQ(pos, QPoint(99, 99));
    }

    {
        cvp->screenNum = 2;
        fakeRet = false;
        it.clear();
        QPoint pos(99, 99);
        EXPECT_TRUE(cvp->itemGridpos(file, pos));
        EXPECT_EQ(it, file);
        EXPECT_EQ(pos, QPoint(9, 9));
        EXPECT_EQ(cvp->overlapPos(), pos);
    }
}

TEST_F(TestCanvasViewPrivate, visualItem)
{
    cvp->screenNum = 1;
    stub_ext::StubExt stub;
    const QString file = "/var/desktop-test";
    stub.set_lamda(&CanvasGrid::item, [file](CanvasGrid *,
                   int index, const QPoint &pos){
        if (index == 1 && pos == QPoint(2, 2))
            return file;
        if (index == 1 && pos == QPoint(9, 9))
            return QString("notoverlap");
        return QString();
    });

    stub.set_lamda(&CanvasGrid::overloadItems, [file](CanvasGrid *, int num){
        if (num == 2)
            return QStringList{QString("1"), QString("2"), file};
        return QStringList();
    });

    EXPECT_TRUE(cvp->visualItem(QPoint(1, 1)).isEmpty());
    cvp->screenNum = 2;
    EXPECT_TRUE(cvp->visualItem(QPoint(2, 2)).isEmpty());
    cvp->screenNum = 1;
    EXPECT_EQ(cvp->visualItem(QPoint(2, 2)), file);

    cvp->screenNum = 1;
    EXPECT_EQ(cvp->visualItem(cvp->overlapPos()), QString("notoverlap"));
    cvp->screenNum = 2;
    EXPECT_EQ(cvp->visualItem(cvp->overlapPos()), file);
}

namespace {

class TestCanvasView : public testing::Test
{
public:
    void SetUp() override {
        fmodel = new FileInfoModel;
        fmodel->d->fileProvider->setRoot(QUrl::fromLocalFile("/tmp"));
        model = new CanvasProxyModel;
        model->setSourceModel(fmodel);

        view = new CanvasView;
        view->setModel(model);

        sel = new CanvasSelectionModel(model, nullptr);
        view->setSelectionModel(sel);

        dlgt = new CanvasItemDelegate(view);
        view->setItemDelegate(dlgt);

        auto in1 = QUrl::fromLocalFile("/tmp/desktop-test-1.txt");
        auto in2 = QUrl::fromLocalFile("/tmp/desktop-test-2.txt");
        fmodel->d->fileList.append(in1);
        fmodel->d->fileList.append(in2);
        model->d->fileList.append(in1);
        model->d->fileList.append(in2);

        DFMSyncFileInfoPointer info1(new SyncFileInfo(in1));
        DFMSyncFileInfoPointer info2(new SyncFileInfo(in2));
        fmodel->d->fileMap.insert(in1, info1);
        fmodel->d->fileMap.insert(in2, info2);
        model->d->fileMap.insert(in1, info1);
        model->d->fileMap.insert(in2, info2);

        cvp = view->d.data();
        cvp->gridMargins = QMargins(2, 2, 2, 2);
        cvp->viewMargins = QMargins(5, 5, 5, 5);
        cvp->canvasInfo = CanvasViewPrivate::CanvasInfo(10, 10, 50, 50);
    }
    void TearDown() override {
        delete view;
        delete sel;
        delete model;
        delete fmodel;
    }
    CanvasProxyModel *model;
    FileInfoModel *fmodel;
    CanvasView *view;
    CanvasItemDelegate *dlgt;
    CanvasSelectionModel *sel;
    CanvasViewPrivate *cvp;
};

}

TEST_F(TestCanvasView, refresh)
{
    stub_ext::StubExt stub;
    bool refresh = false;
    stub.set_lamda(&CanvasProxyModel::refresh,[&refresh](){
        refresh = true;
    });
    bool filcker = false;
    stub.set_lamda((void (CanvasView::*)())&CanvasView::repaint,[&filcker](CanvasView *self){
        filcker = self->d->flicker;
    });

    view->refresh(true);
    EXPECT_TRUE(refresh);
    EXPECT_FALSE(filcker);

    refresh = false;
    filcker = false;
    view->refresh(false);
    EXPECT_TRUE(refresh);
    EXPECT_TRUE(filcker);
}

TEST_F(TestCanvasView, firstIndex)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasGrid::gridCount, [this](CanvasGrid *, int num) {
        return cvp->canvasInfo.gridCount();
    });

    int posCount = 0;
    stub.set_lamda(&CanvasGrid::item, [this, &posCount](CanvasGrid *, int index, const QPoint &pos) {
        posCount++;
        if (index == 2) {
            if (pos == QPoint(9, 8))
                return model->d->fileList.last().toString();
            else if (pos == QPoint(9, 9))
                return model->d->fileList.first().toString();
        }

        return QString();
    });

    {
        cvp->screenNum = 1;
        EXPECT_FALSE(cvp->firstIndex().isValid());
        EXPECT_EQ(posCount, 100);
    }

    {
        posCount = 0;
        cvp->screenNum = 2;
        auto idx = cvp->firstIndex();
        EXPECT_EQ(idx, model->index(1));
        EXPECT_EQ(posCount, 99);
    }
}

TEST_F(TestCanvasView, lastIndex)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasGrid::gridCount, [this](CanvasGrid *, int num) {
        return cvp->canvasInfo.gridCount();
    });

    int posCount = 0;
    stub.set_lamda(&CanvasGrid::item, [this, &posCount](CanvasGrid *, int index, const QPoint &pos) {
        posCount++;
        if (index == 2) {
            if (pos == QPoint(0, 0))
                return model->d->fileList.last().toString();
            else if (pos == QPoint(0, 1))
                return model->d->fileList.first().toString();
        }

        return QString();
    });

    QStringList overlap;
    stub.set_lamda(&CanvasGrid::overloadItems, [&overlap](CanvasGrid *, int num) {
        if (num == 2)
            return overlap;
        return QStringList();
    });

    {
        cvp->screenNum = 1;
        EXPECT_FALSE(cvp->lastIndex().isValid());
        EXPECT_EQ(posCount, 100);
    }

    {
        posCount = 0;
        cvp->screenNum = 2;
        auto idx = cvp->lastIndex();
        EXPECT_EQ(idx, model->index(0));
        EXPECT_EQ(posCount, 99);
    }

    overlap.append("overlap");
    overlap.append(model->d->fileList.first().toString());
    {
        posCount = 0;
        cvp->screenNum = 2;
        auto idx = cvp->lastIndex();
        EXPECT_EQ(idx, model->index(0));
        EXPECT_EQ(posCount, 0);
    }
}

TEST_F(TestCanvasView, findIndex)
{
    stub_ext::StubExt stub;
    QPoint gridAt(1, 1);
    stub.set_lamda(&CanvasViewPrivate::gridAt, [&gridAt]() {
        return gridAt;
    });

    stub.set_lamda(&CanvasViewPrivate::visualItem, [this](CanvasViewPrivate *, const QPoint &gridPos) {
        if (gridPos == QPoint(1, 2))
            return model->d->fileList.first().toString();
        else if (gridPos == QPoint(2, 2))
            return model->d->fileList.last().toString();
        return QString();
    });

    stub.set_lamda(VADDR(CanvasProxyModel, data), [this]
                   (CanvasProxyModel *, const QModelIndex &index, int itemRole) -> QVariant{
        EXPECT_EQ(itemRole, Global::ItemRoles::kItemFilePinyinNameRole);
        return model->d->fileList.at(index.row()).fileName();
    });

    EXPECT_FALSE(cvp->findIndex("m", true, QModelIndex(), false, false).isValid());
    EXPECT_FALSE(cvp->findIndex("desk1", true, QModelIndex(), false, false).isValid());

    {
        auto idx = cvp->findIndex("desk", true, QModelIndex(), false, false);
        EXPECT_EQ(idx, model->index(0));

        idx = cvp->findIndex("desk", true, model->index(0), false, false);
        EXPECT_EQ(idx, model->index(0));

        gridAt = QPoint(1, 4);
        idx = cvp->findIndex("desk", true, model->index(0), false, false);
        EXPECT_EQ(idx, model->index(1));
    }

    {
        gridAt = QPoint(1, 4);
        auto idx = cvp->findIndex("desktop-test-1.txt", false, QModelIndex(), false, false);
        EXPECT_EQ(idx, model->index(0));

        idx = cvp->findIndex("desktop-test-1.txt", false, model->index(0), false, false);
        EXPECT_EQ(idx, model->index(0));
    }

    {
        gridAt = QPoint(1, 4);
        auto idx = cvp->findIndex("desk", false, QModelIndex(), true, false);
        EXPECT_EQ(idx, model->index(1));

        idx = cvp->findIndex("desk", false, model->index(0), true, false);
        EXPECT_EQ(idx, model->index(0));
    }

    {
        gridAt = QPoint(1, 2);
        auto idx = cvp->findIndex("desk", true, model->index(0), false, false);
        EXPECT_EQ(idx, model->index(0));

        idx = cvp->findIndex("desk", true, model->index(0), false, true);
        EXPECT_EQ(idx, model->index(1));

        gridAt = QPoint(2, 2);
        idx = cvp->findIndex("desk", true, model->index(1), false, false);
        EXPECT_EQ(idx, model->index(1));

        idx = cvp->findIndex("desk", true, model->index(1), false, true);
        EXPECT_EQ(idx, model->index(0));
    }
}

TEST_F(TestCanvasView, indexAt)
{
    auto in3 = QUrl::fromLocalFile("/tmp/desktop-test-3.txt");
    fmodel->d->fileList.append(in3);
    model->d->fileList.append(in3);

    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    fmodel->d->fileMap.insert(in3, info3);
    model->d->fileMap.insert(in3, info3);

    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasView::isPersistentEditorOpen, [this](QAbstractItemView *, const QModelIndex &index){
        return model->index(0) == index;
    });
    QWidget editor;
    editor.setGeometry(cvp->visualRect(QPoint(0, 0)).marginsAdded(QMargins(0, 0, 0, 20)));
    stub.set_lamda(&CanvasView::indexWidget, [&editor](){
        return &editor;
    });

    QModelIndex expend;
    stub.set_lamda(&CanvasItemDelegate::mayExpand, [&expend](CanvasItemDelegate *, QModelIndex *who){
        *who = expend;
        return expend.isValid();
    });

    stub.set_lamda(&CanvasView::itemPaintGeomertys, [this](CanvasView *, const QModelIndex &index){
        QList<QRect> ret;
        if (model->index(0) == index) {
            ret.append(cvp->visualRect(QPoint(0, 0)));
        } else if (model->index(1) == index) {
            ret.append(cvp->visualRect(QPoint(1, 0)).marginsAdded(QMargins(0, 0, 0, 20)));
        } else if (model->index(2) == index) {
            ret.append(cvp->visualRect(QPoint(2, 0)).marginsRemoved(QMargins(0, 0, 0, 20)));
        }
        return ret;
    });

    stub.set_lamda(&CanvasViewPrivate::visualItem, [this](CanvasViewPrivate *, const QPoint &gridPos){
        if (gridPos  == QPoint(0, 0)) {
            return model->fileUrl(model->index(0)).toString();
        } else if (gridPos == QPoint(1, 0)) {
            return model->fileUrl(model->index(1)).toString();
        } else if (gridPos == QPoint(2, 0)) {
            return model->fileUrl(model->index(2)).toString();
        }
        return QString();
    });

    // with editor
    {
        expend = QModelIndex();
        view->setCurrentIndex(model->index(0));
        // on view margins
        EXPECT_FALSE(view->indexAt(QPoint(4, 4)).isValid());

        // on item
        EXPECT_EQ(view->indexAt(QPoint(5, 5)), model->index(0));

        // on expended area of editor.
        EXPECT_EQ(view->indexAt(QPoint(10, 70)), model->index(0));

        // out of expended area of editor.
        EXPECT_FALSE(view->indexAt(QPoint(10, 80)).isValid());

        // on other item
        EXPECT_EQ(view->indexAt(QPoint(60, 10)), model->index(1));
    }

    // with expended item
    {
        expend = model->index(1);

        // on item
        view->setCurrentIndex(model->index(1));
        EXPECT_EQ(view->indexAt(QPoint(60, 10)), model->index(1));

        view->setCurrentIndex(QModelIndex());
        EXPECT_EQ(view->indexAt(QPoint(60, 10)), model->index(1));

        // on expend area
        EXPECT_EQ(view->indexAt(QPoint(60, 70)), model->index(1));

        // out of expend area
        EXPECT_FALSE(view->indexAt(QPoint(60, 80)).isValid());

        // on other item
        EXPECT_EQ(view->indexAt(QPoint(10, 10)), model->index(0));
    }

    {
        expend = QModelIndex();
        view->setCurrentIndex(QModelIndex());

        EXPECT_FALSE(view->indexAt(QPoint(110, 4)).isValid());
        EXPECT_EQ(view->indexAt(QPoint(110, 5)), model->index(2));
        EXPECT_EQ(view->indexAt(QPoint(110, 34)), model->index(2));
        EXPECT_FALSE(view->indexAt(QPoint(110, 35)).isValid());
        EXPECT_FALSE(view->indexAt(QPoint(110, 60)).isValid());
    }
}

TEST_F(TestCanvasView, isIndexHidden)
{
    EXPECT_FALSE(view->isIndexHidden(model->index(0)));
    EXPECT_FALSE(view->isIndexHidden(model->index(1)));
    EXPECT_FALSE(view->isIndexHidden(model->index(2)));
    EXPECT_FALSE(view->isIndexHidden(QModelIndex()));
}

TEST_F(TestCanvasView, paintEvent)
{
    stub_ext::StubExt stub;
    view->d->screenNum = 1;
    stub.set_lamda(&CanvasView::itemPaintGeomertys, [this](CanvasView *, const QModelIndex &index){
        QList<QRect> ret;
        if (model->index(0) == index) {
            ret.append(cvp->visualRect(QPoint(0, 0)));
        } else if (model->index(1) == index) {
            ret.append(cvp->visualRect(QPoint(1, 0)));
        }
        return ret;
    });

    QHash<QString, QPoint> itemPos;
    stub.set_lamda(&CanvasGrid::item, [&itemPos](CanvasGrid *, int index, const QPoint &pos) {
        return itemPos.key(pos);
    });

    QStringList overload;
    stub.set_lamda(&CanvasGrid::overloadItems, [&overload](){
        return overload;
    });

    stub.set_lamda(&CanvasGrid::points, [&itemPos](){
        return itemPos;
    });

    QModelIndex expend;
    stub.set_lamda(&CanvasItemDelegate::mayExpand, [&expend](CanvasItemDelegate *, QModelIndex *who){
        *who = expend;
        return expend.isValid();
    });

    QList<QPoint> painted;
    stub.set_lamda(&ViewPainter::drawFile, [&painted](ViewPainter *,
                   QStyleOptionViewItem, const QModelIndex &, const QPoint &gridPos){

        painted.append(gridPos);
    });

    QPixmap pix(600, 600);
    stub.set_lamda(&QAbstractItemView::viewport, [&pix]() -> QWidget * {
        return (QWidget *)((char *)&pix - sizeof(QObject));
    });

    {
        itemPos.insert(model->fileUrl(model->index(0)).toString(), QPoint(0, 0));
        itemPos.insert(model->fileUrl(model->index(1)).toString(), QPoint(2, 0));

        {
            QPaintEvent e(QRect(0, 0, 500, 500));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 2);
            EXPECT_TRUE(painted.contains(QPoint(0, 0)));
            EXPECT_TRUE(painted.contains(QPoint(2, 0)));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(0, 0, 104, 500));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 1);
            EXPECT_EQ(painted.first(), QPoint(0, 0));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(105, 0, 400, 400));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 1);
            EXPECT_EQ(painted.first(), QPoint(2, 0));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(55, 0, 50, 400));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 0);
        }
    }

    // overlap
    {
        itemPos.clear();
        itemPos.insert(model->fileUrl(model->index(1)).toString(), QPoint(2, 0));
        overload.append(model->fileUrl(model->index(0)).toString());

        {
            painted.clear();
            QPaintEvent e(QRect(0, 0, 510, 510));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 2);
            EXPECT_EQ(painted.first(), QPoint(2, 0));
            EXPECT_EQ(painted.last(), QPoint(9, 9));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(0, 0, 454, 454));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 1);
            EXPECT_EQ(painted.first(), QPoint(2, 0));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(455, 455, 20, 20));
            view->paintEvent(&e);
            ASSERT_EQ(painted.size(), 1);
            EXPECT_EQ(painted.first(), QPoint(9, 9));
        }

        {
            painted.clear();
            QPaintEvent e(QRect(155, 155, 200, 200));
            view->paintEvent(&e);
            EXPECT_EQ(painted.size(), 0);
        }
    }
}

TEST_F(TestCanvasView, itemRect)
{
    QHash<QString, QPoint> itemPos;
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasViewPrivate::itemGridpos, [&itemPos](CanvasViewPrivate *, const QString &item, QPoint &gridPos) {
        if (itemPos.contains(item)) {
            gridPos = itemPos.value(item);
            return true;
        }
        return false;
    });

    itemPos.insert(model->fileUrl(model->index(0)).toString(), QPoint(0, 0));
    itemPos.insert(model->fileUrl(model->index(1)).toString(), QPoint(2, 0));

    EXPECT_EQ(view->itemRect(QModelIndex()), QRect());
    EXPECT_EQ(view->itemRect(model->index(0)), QRect(7, 7, 46, 46));
    EXPECT_EQ(view->itemRect(model->index(1)), QRect(107, 7, 46, 46));
}

TEST_F(TestCanvasView, wheelEvent)
{
    stub_ext::StubExt stub;
    int in = 0;
    stub.set_lamda(&CanvasViewMenuProxy::changeIconLevel, [&in](CanvasViewMenuProxy *, bool increase) {
        in = increase ? 1 : -1;
    });

    Qt::KeyboardModifiers ctrl = Qt::NoModifier;
    stub.set_lamda(&QGuiApplication::queryKeyboardModifiers, [&ctrl]() {
        return ctrl;
    });

    {
        QWheelEvent e(QPointF(0,0), -1, {}, {});
        view->wheelEvent(&e);
        EXPECT_EQ(in, 0);
    }

    ctrl = Qt::ControlModifier;
    {
        QWheelEvent e(QPointF(0,0), -1, {}, {});
        view->wheelEvent(&e);
        EXPECT_EQ(in, -1);
    }

    {
        QWheelEvent e(QPointF(0,0), 1, {}, {});
        view->wheelEvent(&e);
        EXPECT_EQ(in, 1);
    }
}

TEST_F(TestCanvasView, selectAll)
{
    sel->clear();
    view->selectAll();

    auto idxes = sel->selectedIndexes();
    ASSERT_EQ(idxes.size(), 2);
}

TEST_F(TestCanvasView, setGeometry)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasItemDelegate::updateItemSizeHint, [](CanvasItemDelegate *self){
        self->d->itemSizeHint = QSize(40, 40);
    });

    QModelIndex cidx;
    stub.set_lamda(&QAbstractItemView::closePersistentEditor, [&cidx](QAbstractItemView *self, const QModelIndex &index){
        cidx = index;
    });
    stub.set_lamda(&QAbstractItemView::isPersistentEditorOpen, [](){
        return true;
    });

    QMargins inMargins;
    QSize inViewSize;
    QSize inItemSize;
    stub.set_lamda(&CanvasViewPrivate::updateGridSize, [&inMargins, &inViewSize, &inItemSize](CanvasViewPrivate *self,
                   const QSize &viewSize,
                   const QMargins &geometryMargins, const QSize &itemSize){
        inMargins = geometryMargins;
        inViewSize = viewSize;
        inItemSize = itemSize;
        self->canvasInfo.columnCount = 9;
        self->canvasInfo.rowCount = 12;
        return;
    });

    bool grid = false;
    stub.set_lamda(&CanvasGrid::updateSize, [&grid, this](CanvasGrid *, int index, const QSize &size){
        EXPECT_EQ(index, view->d->screenNum);
        EXPECT_EQ(size, QSize(9, 12));
        grid = true;
    });

    view->setCurrentIndex(model->index(0));
    view->d->screenNum = 23;
    view->setGeometry(QRect(0, 0, 998, 998));
    EXPECT_EQ(dlgt->d->itemSizeHint, QSize(40, 40));
    EXPECT_EQ(cidx, model->index(0));
    EXPECT_EQ(inMargins, QMargins(0, 0, 0, 0));
    EXPECT_EQ(inViewSize, QSize(998, 998));
    EXPECT_EQ(inItemSize, dlgt->d->itemSizeHint);
    EXPECT_TRUE(grid);
}

TEST_F(TestCanvasView, moveCursor)
{
    auto in3 = QUrl::fromLocalFile("/tmp/desktop-test-3.txt");
    auto in4 = QUrl::fromLocalFile("/tmp/desktop-test-4.txt");
    auto in5 = QUrl::fromLocalFile("/tmp/desktop-test-5.txt");
    auto in6 = QUrl::fromLocalFile("/tmp/desktop-test-6.txt");
    fmodel->d->fileList.append(in3);
    model->d->fileList.append(in3);
    fmodel->d->fileList.append(in4);
    model->d->fileList.append(in4);
    fmodel->d->fileList.append(in5);
    model->d->fileList.append(in5);
    fmodel->d->fileList.append(in6);
    model->d->fileList.append(in6);

    DFMSyncFileInfoPointer info3(new SyncFileInfo(in3));
    DFMSyncFileInfoPointer info4(new SyncFileInfo(in4));
    DFMSyncFileInfoPointer info5(new SyncFileInfo(in5));
    DFMSyncFileInfoPointer info6(new SyncFileInfo(in6));
    fmodel->d->fileMap.insert(in3, info3);
    model->d->fileMap.insert(in3, info3);
    fmodel->d->fileMap.insert(in4, info4);
    model->d->fileMap.insert(in4, info4);
    fmodel->d->fileMap.insert(in5, info5);
    model->d->fileMap.insert(in5, info5);
    fmodel->d->fileMap.insert(in6, info6);
    model->d->fileMap.insert(in6, info6);

    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasViewPrivate::firstIndex, [this](){
        return model->index(0);
    });

    stub.set_lamda(&CanvasViewPrivate::lastIndex, [this](){
        return model->index(model->d->fileList.size() - 1);
    });


    QHash<QString, QPoint> itemPos;
    stub.set_lamda(&CanvasGrid::point, [&itemPos](CanvasGrid *, const QString &item, QPair<int, QPoint> &pos){
        if (itemPos.contains(item)) {
            pos.first = 1;
            pos.second = itemPos.value(item);
            return true;
        }
        return false;
    });

    stub.set_lamda(&CanvasGrid::item, [&itemPos](CanvasGrid *, int index, const QPoint &pos){
        EXPECT_EQ(index, 1);
        return itemPos.key(pos);
    });

    view->d->screenNum = 1;
    itemPos.insert(model->fileUrl(model->index(0)).toString(), QPoint(0, 5));
    itemPos.insert(model->fileUrl(model->index(1)).toString(), QPoint(3, 1));
    itemPos.insert(model->fileUrl(model->index(2)).toString(), QPoint(5, 5));
    itemPos.insert(model->fileUrl(model->index(3)).toString(), QPoint(3, 7));
    itemPos.insert(model->fileUrl(model->index(4)).toString(), QPoint(3, 5));
    itemPos.insert(model->fileUrl(model->index(5)).toString(), QPoint(9, 9));

    {
        view->setCurrentIndex(QModelIndex());
        auto target = model->index(0);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveLeft, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveRight, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePrevious, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveUp, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveNext, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveHome, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePageUp, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveEnd, Qt::NoModifier), target);
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePageDown, Qt::NoModifier), target);
    }

    {
        view->setCurrentIndex(model->index(4));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveLeft, Qt::NoModifier)
                  , model->index(0));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveRight, Qt::NoModifier)
                  , model->index(2));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePrevious, Qt::NoModifier)
                  , model->index(1));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveUp, Qt::NoModifier)
                  , model->index(1));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveNext, Qt::NoModifier)
                  , model->index(3));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier)
                  , model->index(3));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveHome, Qt::NoModifier)
                  , model->index(0));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePageUp, Qt::NoModifier)
                  , model->index(0));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveEnd, Qt::NoModifier)
                  , model->index(5));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePageDown, Qt::NoModifier)
                  , model->index(5));
    }

    {
        view->setCurrentIndex(model->index(1));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePrevious, Qt::NoModifier)
                  , model->index(0));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveUp, Qt::NoModifier)
                  , model->index(0));
    }

    {
        view->setCurrentIndex(model->index(2));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveNext, Qt::NoModifier)
                  , model->index(5));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier)
                  , model->index(5));
    }

    // overlap
    auto in7 = QUrl::fromLocalFile("/tmp/desktop-test-7.txt");
    fmodel->d->fileList.append(in7);
    model->d->fileList.append(in7);
    DFMSyncFileInfoPointer info7(new SyncFileInfo(in7));
    fmodel->d->fileMap.insert(in7, info7);
    model->d->fileMap.insert(in7, info7);

    {
        view->setCurrentIndex(model->index(4));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveEnd, Qt::NoModifier)
                  , model->index(6));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MovePageDown, Qt::NoModifier)
                  , model->index(6));
    }

    {
        view->setCurrentIndex(model->index(2));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveNext, Qt::NoModifier)
                  , model->index(6));
        EXPECT_EQ(view->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier)
                  , model->index(6));
    }
}
