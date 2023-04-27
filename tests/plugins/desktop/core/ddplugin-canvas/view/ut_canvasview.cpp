// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/canvasitemdelegate_p.h"
#include "view/canvasview_p.h"
#include "model/fileinfomodel_p.h"
#include "model/canvasproxymodel_p.h"
#include "model/canvasselectionmodel.h"
#include "displayconfig.h"
#include "watermask/deepinlicensehelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/syncfileinfo.h"

#include "stubext.h"

#include <gtest/gtest.h>

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

    EXPECT_EQ(view.selectionMode(), QAbstractItemView::NoSelection);
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
