// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/viewpainter.h"
#include "grid/canvasgrid.h"
#include "view/operator/boxselector.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>


using namespace ddplugin_canvas;

class UT_ViewPainter : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        view = new CanvasView();
        viewP = new CanvasViewPrivate(view);
        painter = new ViewPainter(viewP);

    }
    virtual void TearDown() override
    {
        delete painter;
        delete viewP;
        delete view;

        stub.clear();

    }
    CanvasView *view = nullptr;
    CanvasViewPrivate *viewP = nullptr;
    ViewPainter *painter = nullptr;
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

TEST_F(UT_ViewPainter, drawDodge)
{
    bool prepareDoge = true;
    stub.set_lamda(&DodgeOper::getPrepareDodge,[&prepareDoge](){
        __DBG_STUB_INVOKE__
                return prepareDoge;
    });

    bool dodgeAnimationing = true;
    stub.set_lamda(&DodgeOper::getDodgeAnimationing,[&dodgeAnimationing](){
        __DBG_STUB_INVOKE__
                return dodgeAnimationing;
    });

    stub.set_lamda(&CanvasProxyModel::fileUrl,[](CanvasProxyModel *self, const QModelIndex&){
        __DBG_STUB_INVOKE__
                return QUrl("file://temp1");
    });
    stub.set_lamda(&CanvasSelectionModel::selectedUrls,[](){
        QList<QUrl> res;
        res.push_back(QUrl("file://temp2"));
        return res;
    });


    typedef bool(*fun_type)();
    stub.set_lamda((fun_type)(&QPersistentModelIndex::isValid),[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    bool call = false;
    stub.set_lamda(&DodgeOper::getDodgeItems,[&call](){
        __DBG_STUB_INVOKE__
                QList<QString> res{QString("temp_str1"),QString("temp_str2")};
                call = true;
                return res;
    });
    TestItemModel testi;
    QModelIndex rootIndex = QModelIndex(1, 1, nullptr,&testi);
    painter->d->dragDropOper->dragHoverIndex = QPersistentModelIndex(rootIndex);
    painter->d->dodgeOper->dodgeItems.push_back(QString("temp_str1"));
    painter->d->dodgeOper->dodgeItems.push_back(QString("temp_str2"));

    viewP->dodgeOper = new DodgeOper(view);
    CanvasProxyModel proxyModel ;
    view->setModel(&proxyModel);
    QStyleOptionViewItem option;
    painter->drawDodge(option);
    EXPECT_TRUE(call);
}

TEST_F(UT_ViewPainter, polymerize)
{
    QModelIndexList indexs;
    TestItemModel testItem;
    QModelIndex index1(1,1,nullptr,&testItem);
    QModelIndex index2(2,2,nullptr,&testItem);
    indexs.push_back(index1);
    indexs.push_back(index2);

    stub.set_lamda(&OperState::current,[&index1](){
        __DBG_STUB_INVOKE__
                return index1;
    });
    CanvasView canvasview ;
    CanvasItemDelegate item(&canvasview);
    stub.set_lamda(&CanvasView::itemDelegate,[&item](){
        __DBG_STUB_INVOKE__
                return &item;
    });

    stub.set_lamda(&CanvasItemDelegate::paintDragIcon,
                   [](CanvasItemDelegate *self, QPainter*,const QStyleOptionViewItem&,const QModelIndex &){
        __DBG_STUB_INVOKE__
                return QSize(1,1);
    });

    bool callDrawDragText = false;
    stub.set_lamda(&ViewPainter::drawDragText,[&callDrawDragText](QPainter *painter, const QString &str, const QRect &rect){
        __DBG_STUB_INVOKE__
                callDrawDragText = true;
    });

    bool calldrawEllipseBackground= false;
    stub.set_lamda(&ViewPainter::drawEllipseBackground,[&calldrawEllipseBackground](QPainter *painter, const QRect &rect){
        __DBG_STUB_INVOKE__
                calldrawEllipseBackground = true;
    });
    QPixmap res = painter->polymerize(indexs,viewP);
    EXPECT_EQ(res.rect(),QRect(0,0,188,188));
    EXPECT_EQ(res.devicePixelRatio(),1);
    EXPECT_TRUE(callDrawDragText);
    EXPECT_TRUE(calldrawEllipseBackground);
}

TEST_F(UT_ViewPainter, drawDragText)
{
    QPainter paint;
    const QString str = QString("temp_str");
    const QRect rect  = QRect(1,1,2,2);

    painter->drawDragText(&paint,str,rect);

    EXPECT_EQ(paint.font().pixelSize(),-1);
    EXPECT_TRUE(paint.font().Bold);
}

TEST_F(UT_ViewPainter, drawEllipseBackground)
{
    QPainter paint;
    const QRect rect  = QRect(1,1,2,2);
    painter->drawEllipseBackground(&paint,rect);
    QColor pointColor(244, 74, 74);
    EXPECT_FALSE(paint.renderHints() & QPainter::Antialiasing);
    EXPECT_EQ(paint.opacity(),1);
}
