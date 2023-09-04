// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/dodgeoper.h"
#include "private/canvasmanager_p.h"
#include "canvasmanager.h"
#include "utils/keyutil.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"


#include "dfm-base/interfaces/fileinfo.h"

#include <gtest/gtest.h>
#include "stubext.h"

#include <QDragMoveEvent>
#include <QMimeData>

using namespace ddplugin_canvas;

class UT_DodgeOper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        parent = new CanvasView();
        qq = new DodgeOper(parent);
        manager = new CanvasManager();
        managerP = new CanvasManagerPrivate(manager);
        manager->d = managerP;

    }
    virtual void TearDown() override
    {
        delete managerP;
        delete manager;
        delete qq;
        delete parent;
        stub.clear();
    }
    DodgeOper *qq = nullptr;
    CanvasView *parent = nullptr;
    stub_ext::StubExt stub;
    CanvasManager *manager =nullptr;
    CanvasManagerPrivate *managerP = nullptr;

};

TEST_F(UT_DodgeOper, updatePrepareDodgeValue)
{
    stub.set_lamda(&ddplugin_canvas::isCtrlPressed,[](){
        __DBG_STUB_INVOKE__
        return false;
    });
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    event.t = QEvent::DragEnter;
    QMimeData mimedata;
    event.mdata = &mimedata;
    qq->updatePrepareDodgeValue(&event);
    EXPECT_FALSE(qq->prepareDodge);
    stub.set_lamda(&QDropEvent::source,[this](){
        __DBG_STUB_INVOKE__
        return parent;
    });
    qq->updatePrepareDodgeValue(&event);
    EXPECT_TRUE(qq->prepareDodge);
}

TEST_F(UT_DodgeOper, tryDodge)
{
    stub.set_lamda(&QDropEvent::source,[this](){
        __DBG_STUB_INVOKE__
        return parent;
    });

    stub.set_lamda(&CanvasGrid::point,[](CanvasGrid*,const QString &item, QPair<int, QPoint> &pos){
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGrid::item,[](CanvasGrid*,int index, const QPoint &pos){
        __DBG_STUB_INVOKE__
        return QString("temp_item");
    });
    stub.set_lamda(&CanvasGrid::gridCount,[](CanvasGrid*,int index){
        __DBG_STUB_INVOKE__
        return 2;
    });
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    qq->dodgeAnimationing = false;
    QMimeData mimdata;
    QList<QUrl> urls{QUrl("temp1"),QUrl("temp2")};
    mimdata.setUrls(urls);
    event.mdata = &mimdata;

    qq->tryDodge(&event);

    EXPECT_TRUE(qq->dodgeDelayTimer.isActive());
}

TEST_F(UT_DodgeOper, startDodgeAnimation)
{
    stub.set_lamda(&DodgeOper::calcDodgeTargetGrid,[](){
        __DBG_STUB_INVOKE__
        return true;
    });
    qq->startDodgeAnimation();
    EXPECT_EQ(qq->animation->duration(),300);
    EXPECT_EQ(qq->animation->startValue(),0.0);
    EXPECT_EQ(qq->animation->endValue(),1.0);
}

TEST_F(UT_DodgeOper, dodgeAnimationFinished)
{
    qq->dodgeAnimationFinished();
    EXPECT_FALSE(qq->dodgeAnimationing);
}

TEST_F(UT_DodgeOper, calcDodgeTargetGrid)
{
    CanvasProxyModel proxyModel;
    CanvasSelectionModel canvasselectmodel(&proxyModel,nullptr);
    stub.set_lamda(&CanvasView::selectionModel,[&canvasselectmodel](){
        __DBG_STUB_INVOKE__
        return &canvasselectmodel;
    });

    stub.set_lamda(&DodgeItemsOper::tryDodge,[](DodgeItemsOper *,const QStringList &orgItems, const GridPos &ref, QStringList &dodgeItems){
        __DBG_STUB_INVOKE__
         return true;
    });

    EXPECT_TRUE(qq->calcDodgeTargetGrid());
    EXPECT_TRUE(qq->dodgeItems.isEmpty());
}

class UT_DodgeItemsOper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        core = new GridCore;
        qq = new DodgeItemsOper(core);
    }
    virtual void TearDown() override
    {

        delete qq;
        delete core;
        stub.clear();
    }
    DodgeItemsOper *qq = nullptr;
    GridCore *core = nullptr;

    stub_ext::StubExt stub;
};

TEST_F(UT_DodgeItemsOper, tryDodge)
{
    auto fun_type = static_cast<QList<int>(DodgeItemsOper::*)(const int,const QList<QPoint>&)>(&DodgeItemsOper::toIndex);
    stub.set_lamda(fun_type,[](DodgeItemsOper*,const int ,const QList<QPoint>&){
        __DBG_STUB_INVOKE__
                return QList{1,1};
    });
    stub.set_lamda(&DodgeItemsOper::toPos,[](DodgeItemsOper*,const int,const int){
        __DBG_STUB_INVOKE__
        return QPoint(1,1);
    });
    QStringList orgItems{"org_temp1","org_temp2"};
    QStringList dodgeItems{"dodge_temp1","dodge_temp2"};
    GridPos ref(1,QPoint(1,1));

    QHash<QString, QPoint> item;
    item.insert("org_temp1",QPoint(1,1));
    item.insert("org_temp2",QPoint(2,2));
    qq->itemPos.insert(0,item);
    EXPECT_TRUE(qq->tryDodge(orgItems,ref,dodgeItems));
    EXPECT_EQ(qq->itemPos[1].value("org_temp1"),QPoint(1,1));
    EXPECT_EQ(qq->itemPos[1].value("org_temp2"),QPoint(1,1));
}

TEST_F(UT_DodgeItemsOper, findEmpty)
{
    auto fun_type = static_cast<QList<int>(DodgeItemsOper::*)(const int,const QList<QPoint>&)>(&DodgeItemsOper::toIndex);
    stub.set_lamda(fun_type,[](DodgeItemsOper*,const int ,const QList<QPoint>&){
        __DBG_STUB_INVOKE__
                return QList{1,3,4};
    });
    int res = qq->findEmptyBackward(1,1,0);
    EXPECT_EQ(res,1);

    qq->surfaces.insert(1,QSize(1,1));
    res = qq->findEmptyBackward(1,2,2);
    EXPECT_EQ(res,4);

    res = qq->findEmptyForward(1,2,2);
    EXPECT_EQ(res,0);
}



TEST_F(UT_DodgeItemsOper, reloach)
{
    stub.set_lamda(&DodgeItemsOper::toPos,[](DodgeItemsOper*,const int,const int){
        __DBG_STUB_INVOKE__
        return QPoint(1,1);
    });
   QStringList res = qq->reloachForward(1,1,1);
   EXPECT_EQ(res.size(),0);

   QHash<QPoint,QString> pos;
   pos.insert(QPoint(1,1),"temp_str");
   qq->posItem.insert(1,pos);

   qq->surfaces.insert(1,QSize(2,2));
   res = qq->reloachForward(1,1,2);

   EXPECT_EQ(res,QList{QString("temp_str")});

   qq->posItem.clear();
   qq->posItem.insert(1,pos);
   res = qq->reloachBackward(1,1,2);

   EXPECT_EQ(res,QList{QString("temp_str")});

}

