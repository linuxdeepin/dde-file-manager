// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/canvasitemdelegate_p.h"
#include "view/operator/clickselector.h"
#include "view/canvasview_p.h"
#include "model/fileinfomodel_p.h"
#include "model/canvasproxymodel_p.h"
#include "model/canvasselectionmodel.h"
#include "utils/keyutil.h"

#include "stubext.h"

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_canvas;

TEST(ClickSelector, order)
{
    QPoint p1(1, 1);
    QPoint p2(0, 1);
    QPoint from;
    QPoint to;
    ClickSelector::order(p1, p2, from, to);
    EXPECT_EQ(from, p2);
    EXPECT_EQ(to, p1);

    p1 = QPoint(1, 1);
    p2 = QPoint(3, 1);
    ClickSelector::order(p1, p2, from, to);
    EXPECT_EQ(from, p1);
    EXPECT_EQ(to, p2);

    p1 = QPoint(5, 3);
    p2 = QPoint(5, 5);
    ClickSelector::order(p1, p2, from, to);
    EXPECT_EQ(from, p1);
    EXPECT_EQ(to, p2);

    p1 = QPoint(5, 3);
    p2 = QPoint(9, 5);
    ClickSelector::order(p1, p2, from, to);
    EXPECT_EQ(from, p1);
    EXPECT_EQ(to, p2);

    p1 = QPoint(5, 10);
    p2 = QPoint(9, 5);
    ClickSelector::order(p1, p2, from, to);
    EXPECT_EQ(from, p2);
    EXPECT_EQ(to, p1);
}

TEST(ClickSelector, horizontalTraversal)
{
    QPoint p1(0, 0);
    QPoint p2(1, 1);
    QSize size(2,2);
    auto pos = ClickSelector::horizontalTraversal(p1, p2, size);
    ASSERT_EQ(pos.size(), 4);
    EXPECT_EQ(pos[0], QPoint(0,0));
    EXPECT_EQ(pos[1], QPoint(1,0));
    EXPECT_EQ(pos[2], QPoint(0,1));
    EXPECT_EQ(pos[3], QPoint(1,1));
}

namespace {

class TestClickSelector : public testing::Test
{
public:
    void SetUp() override {
        fmodel = new FileInfoModel;
        fmodel->d->fileProvider->rootUrl = QUrl::fromLocalFile("/tmp");
        model = new CanvasProxyModel;
        model->setSourceModel(fmodel);

        view = new CanvasView;
        view->setSelectionMode(QAbstractItemView::NoSelection);
        view->setModel(model);

        sel = new CanvasSelectionModel(model, nullptr);
        view->setSelectionModel(sel);

        dlgt = new CanvasItemDelegate(view);
        view->setItemDelegate(dlgt);

        auto in1 = QUrl::fromLocalFile("/tmp/1.txt");
        auto in2 = QUrl::fromLocalFile("/tmp/2.txt");
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

        click = view->d->clickSelector;
        view->d->operState().setView(view);
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
    CanvasSelectionModel *sel;
    CanvasItemDelegate *dlgt;
    ClickSelector *click;
};

}

TEST_F(TestClickSelector, click)
{
    bool ctrl = false;
    bool shift = false;
    stub_ext::StubExt stub;
    stub.set_lamda(&isCtrlPressed, [&ctrl](){
        return ctrl;
    });
    stub.set_lamda(&isShiftPressed, [&shift](){
        return shift;
    });


    bool ctu = false;
    stub.set_lamda(&ClickSelector::continuesSelect,[&ctu](){
        ctu = true;
    });

    // clear
    {
        ctu = false;
        auto idx = model->index(0);
        click->lastPressedIndex = idx;
        click->toggleIndex = idx;
        sel->select(idx, QItemSelectionModel::ClearAndSelect);
        view->d->operState().setCurrent(idx);
        view->d->operState().setContBegin(idx);
        ASSERT_FALSE(sel->selectedIndexes().isEmpty());

        click->click(QModelIndex());
        EXPECT_TRUE(sel->selectedIndexes().isEmpty());
        EXPECT_FALSE(view->d->operState().current().isValid());
        EXPECT_FALSE(view->d->operState().contBegin.isValid());
        EXPECT_EQ(click->lastPressedIndex, QModelIndex());
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        EXPECT_FALSE(ctu);
    }

    {
        ctu = false;
        ctrl = true;
        auto idx = model->index(0);
        click->lastPressedIndex = idx;
        click->toggleIndex = idx;
        sel->select(idx, QItemSelectionModel::ClearAndSelect);
        view->d->operState().setCurrent(idx);
        view->d->operState().setContBegin(idx);
        ASSERT_FALSE(sel->selectedIndexes().isEmpty());

        click->click(QModelIndex());
        EXPECT_FALSE(sel->selectedIndexes().isEmpty());
        EXPECT_EQ(click->lastPressedIndex, QModelIndex());
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        EXPECT_FALSE(ctu);
    }

    {
        ctu = false;
        ctrl = false;
        shift = true;
        auto idx = model->index(0);
        click->lastPressedIndex = idx;
        click->toggleIndex = idx;
        sel->select(idx, QItemSelectionModel::ClearAndSelect);
        view->d->operState().setCurrent(idx);
        view->d->operState().setContBegin(idx);
        ASSERT_FALSE(sel->selectedIndexes().isEmpty());

        click->click(QModelIndex());
        EXPECT_FALSE(sel->selectedIndexes().isEmpty());
        EXPECT_EQ(click->lastPressedIndex, QModelIndex());
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        EXPECT_FALSE(ctu);
    }

    // single
    {
        ctu = false;
        ctrl = false;
        shift = false;
        auto idx = model->index(0);
        click->lastPressedIndex = QModelIndex();
        click->toggleIndex = idx;
        sel->clear();
        ASSERT_TRUE(sel->selectedIndexes().isEmpty());

        click->click(idx);
        ASSERT_EQ(sel->selectedIndexes().size(), 1);
        EXPECT_EQ(sel->selectedIndexes().first(), idx);
        EXPECT_EQ(view->d->operState().current(), idx);
        EXPECT_EQ(view->d->operState().contBegin, idx);
        EXPECT_EQ(click->lastPressedIndex, idx);
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        EXPECT_FALSE(ctu);
    }

    // expend
    {
        ctu = false;
        ctrl = true;
        shift = false;

        auto idx = model->index(0);
        click->lastPressedIndex = QModelIndex();
        click->toggleIndex = idx;
        sel->select(idx, QItemSelectionModel::ClearAndSelect);
        view->d->operState().setCurrent(idx);
        view->d->operState().setContBegin(idx);
        ASSERT_FALSE(sel->selectedIndexes().isEmpty());

        idx = model->index(1);
        click->click(idx);
        EXPECT_EQ(click->lastPressedIndex, idx);
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        ASSERT_EQ(sel->selectedIndexes().size(), 2);
        EXPECT_EQ(sel->selectedIndexes().last(), idx);
        EXPECT_EQ(view->d->operState().current(), idx);
        EXPECT_EQ(view->d->operState().contBegin, idx);
        EXPECT_FALSE(ctu);

        click->click(idx);
        EXPECT_EQ(click->toggleIndex, idx);
        ASSERT_EQ(sel->selectedIndexes().size(), 2);
    }

    //continues
    {
        ctu = false;
        ctrl = false;
        shift = true;

        auto idx = model->index(0);
        click->lastPressedIndex = QModelIndex();
        click->toggleIndex = idx;
        sel->clear();
        ASSERT_TRUE(sel->selectedIndexes().isEmpty());

        click->click(idx);
        EXPECT_EQ(click->lastPressedIndex, idx);
        EXPECT_EQ(click->toggleIndex, QModelIndex());
        EXPECT_TRUE(ctu);
    }
}
