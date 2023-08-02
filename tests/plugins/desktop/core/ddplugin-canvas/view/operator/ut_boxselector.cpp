// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/boxselector.h"
#include "view/canvasview_p.h"
#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include "stubext.h"
#include "utils/keyutil.h"
#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
using namespace ddplugin_canvas;

TEST(RubberBand, construct)
{
    RubberBand rb;
    EXPECT_FALSE(rb.autoFillBackground());
    EXPECT_TRUE(rb.testAttribute(Qt::WA_TranslucentBackground));
}

TEST(RubberBand, touch)
{
    stub_ext::StubExt stub;
    bool hide = false;
    stub.set_lamda(&QWidget::hide, [&hide](){
        hide = true;
    });

    RubberBand rb;
    QWidget wid;
    rb.touch(&wid);

    EXPECT_EQ(rb.parentWidget(), &wid);
    EXPECT_TRUE(hide);

    hide = false;
    QWidget wid2;
    rb.touch(&wid2);
    EXPECT_EQ(rb.parentWidget(), &wid2);
    EXPECT_TRUE(hide);
}

TEST(RubberBand, onParentDestroyed)
{
    stub_ext::StubExt stub;
    bool hide = false;
    stub.set_lamda(&QWidget::hide, [&hide](){
        hide = true;
    });

    QWidget wid;
    RubberBand rb;
    rb.setParent(&wid);

    rb.onParentDestroyed(nullptr);
    EXPECT_EQ(rb.parentWidget(), &wid);
    EXPECT_FALSE(hide);

    hide = false;
    rb.onParentDestroyed(&wid);
    EXPECT_EQ(rb.parentWidget(), nullptr);
    EXPECT_TRUE(hide);
}

TEST(RubberBand, paintEvent)
{
    RubberBand rb;
    QPaintEvent e(rb.rect());
    EXPECT_NO_THROW(rb.paintEvent(&e));
}

TEST(BoxSelector, beginSelect)
{
    BoxSelector bs;
    bs.beginSelect(QPoint(1,1), false);

    EXPECT_EQ(bs.begin, QPoint(1,1));
    EXPECT_EQ(bs.end, QPoint(1,1));
    EXPECT_TRUE(bs.active);
    EXPECT_FALSE(bs.automatic);
}

TEST(BoxSelector, set)
{
    BoxSelector bs;
    bool update = false;
    stub_ext::StubExt stub;
    stub.set_lamda(&BoxSelector::delayUpdate, [&update](){
        update = true;
    });

    bs.setAcvite(true);
    EXPECT_TRUE(bs.active);
    EXPECT_TRUE(update);

    update = false;
    bs.setBegin(QPoint(2,2));
    EXPECT_EQ(bs.begin, QPoint(2,2));
    EXPECT_TRUE(update);

    update = false;
    bs.setEnd(QPoint(4,4));
    EXPECT_EQ(bs.begin, QPoint(2,2));
    EXPECT_EQ(bs.end, QPoint(4,4));
    EXPECT_TRUE(update);
}

TEST(BoxSelector, validRect)
{
    BoxSelector bs;
    bs.begin = QPoint(0, 0);
    bs.end = QPoint(1000, 1000);

    CanvasView view;
    view.QAbstractItemView::setGeometry(200, 200, 200, 200);

    EXPECT_EQ(bs.validRect(&view), QRect(0,0,200,200));

    bs.begin = QPoint(300, 300);
    bs.end = QPoint(349, 349);
    EXPECT_EQ(bs.validRect(&view), QRect(100,100,50,50));
}

TEST(BoxSelector, isBeginFrom)
{
    BoxSelector bs;
    bs.begin = QPoint(0, 0);

    CanvasView view;
    view.QAbstractItemView::setGeometry(200, 200, 200, 200);
    EXPECT_FALSE(bs.isBeginFrom(&view));

    bs.begin = QPoint(210, 210);
    EXPECT_TRUE(bs.isBeginFrom(&view));
}

TEST(BoxSelector, endSelect)
{
    stub_ext::StubExt stub;
    bool update = false;
    stub.set_lamda(&BoxSelector::updateRubberBand, [&update](){
        update = true;
    });

    BoxSelector bs;
    bs.active = true;

    bs.endSelect();
    EXPECT_TRUE(update);
    EXPECT_FALSE(bs.active);

    update = false;
    bs.updateTimer.start(1000);
    bs.endSelect();
    EXPECT_FALSE(update);
    EXPECT_FALSE(bs.active);
}

TEST(BoxSelector, eventFilter)
{
    BoxSelector bs;

    stub_ext::StubExt stub;
    bool endSelect = false;
    stub.set_lamda(&BoxSelector::endSelect, [&endSelect](){
        endSelect = true;
    });

    bool update = false;
    stub.set_lamda(&BoxSelector::delayUpdate, [&update](){
        update = true;
    });

    QWidget wid;
    {
        QMouseEvent e(QEvent::MouseButtonRelease, QPoint(0,0), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_FALSE(endSelect);
        EXPECT_FALSE(update);
    }

    bs.active = true;
    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseMove, QPoint(100, 100), QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_FALSE(endSelect);
        EXPECT_TRUE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }

    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseMove, QPoint(110, 110), QPoint(110, 110), Qt::RightButton, Qt::RightButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_TRUE(endSelect);
        EXPECT_FALSE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }

    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseButtonRelease, QPoint(110, 110), QPoint(110, 110), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_TRUE(endSelect);
        EXPECT_FALSE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }
}


class UT_BoxSelector : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        view = new CanvasView;
        proxModel = new CanvasProxyModel();
        manager = new CanvasManager();
        CanvasIns->d->selectionModel = new CanvasSelectionModel(proxModel,nullptr);
        bs = new BoxSelector();
        QUrl url1 = QUrl::fromLocalFile("file:/temp_url1");
        QUrl url2 = QUrl::fromLocalFile("file:/temp_url2");
        view->d->operState().setView(view);
        stub.set_lamda(&CanvasManager::views,[](){
            __DBG_STUB_INVOKE__
                    CanvasView *v = new CanvasView();
            v->d->operState().setView(v);
                QSharedPointer<CanvasView> ptr(v);
                   QList<QSharedPointer<CanvasView>> res{ptr};
                   return res;
        });
        stub.set_lamda(&CanvasView::selectionModel,[](){
            __DBG_STUB_INVOKE__
             CanvasProxyModel *proxyModel = new CanvasProxyModel();

            return new CanvasSelectionModel(proxyModel,nullptr);
        });

    }
    virtual void TearDown() override
    {
        delete view;
        delete manager;
        delete bs;
        stub.clear();
    }
     CanvasView *view;
    CanvasProxyModel *proxModel = nullptr;
    CanvasManager *manager  = nullptr;
    BoxSelector *bs = nullptr;
    stub_ext::StubExt stub ;
};
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
TEST_F(UT_BoxSelector, delayUpdate)
{
    BoxSelector bs;
    bs.updateTimer.id = -1;
    stub_ext::StubExt stub;
    EXPECT_NO_FATAL_FAILURE(bs.delayUpdate());
}

TEST_F(UT_BoxSelector, updateRubberBand)
{
     bs->updateRubberBand();
     EXPECT_FALSE(bs->rubberBand.isVisible());
}
TEST_F(UT_BoxSelector, updateSelection)
{
    bool isCtrl = false;
    bool isShift = false;
    stub.set_lamda(&ddplugin_canvas::isCtrlPressed,[&isCtrl](){
        return isCtrl;
    });
    stub.set_lamda(&ddplugin_canvas::isShiftPressed,[&isShift](){
        return isShift;
    });

    auto fun_type = static_cast<void(BoxSelector::*)(QItemSelection*)>(&BoxSelector::selection);
    stub.set_lamda(fun_type,[](BoxSelector*,QItemSelection *){
        __DBG_STUB_INVOKE__
    });
    EXPECT_NO_FATAL_FAILURE(bs->updateSelection());
    isCtrl = true;
    isShift = false;
    EXPECT_NO_FATAL_FAILURE(bs->updateSelection());
    isCtrl = false;
    isShift = true;
    EXPECT_NO_FATAL_FAILURE(bs->updateSelection());

}

TEST_F(UT_BoxSelector, updateCurrentIndex)
{
    typedef QModelIndex(*fun_type)(const QPoint&);
    TestItemModel testi ;
    QModelIndex resIndex(0, 0, (void *)nullptr,&testi);
    stub.set_lamda((fun_type)(&CanvasView::indexAt),[&resIndex](const QPoint&){
        __DBG_STUB_INVOKE__
        return resIndex;
    });
    stub.set_lamda(&QItemSelectionModel::isSelected,[](){
        __DBG_STUB_INVOKE__
        return true;
    });
    bs->updateCurrentIndex();
    QModelIndex list_index;
    stub.set_lamda(&CanvasSelectionModel::selectedIndexesCache,[list_index](){
        __DBG_STUB_INVOKE__
                QModelIndexList list{list_index};
       return list;
    });


    resIndex = QModelIndex(0, 0, (void *)nullptr, nullptr);
    bs->updateCurrentIndex();
    EXPECT_EQ(view->d->state.view->currentIndex(),list_index);
    EXPECT_EQ(view->d->state.contBegin,list_index);
}

TEST_F(UT_BoxSelector, selection)
{
    QModelIndex leftIndex;
    QModelIndex bottomIndex;
    QItemSelection newSelection(leftIndex,bottomIndex);
    EXPECT_NO_FATAL_FAILURE(bs->selection(&newSelection));

    CanvasProxyModel md;
    stub.set_lamda(&CanvasView::model, [&md]() -> CanvasProxyModel* {
        __DBG_STUB_INVOKE__
        return &md;
    });
    EXPECT_NO_FATAL_FAILURE(bs->selection(view,QRect(1,1,2,2),&newSelection));
}

