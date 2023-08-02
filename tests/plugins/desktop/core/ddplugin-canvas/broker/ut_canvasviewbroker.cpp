// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "broker/canvasviewbroker.h"
#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include "view/canvasview.h"
#include "view/canvasview_p.h"
#include "delegate/canvasitemdelegate_p.h"
#include "model/canvasselectionmodel.h"
#include <dfm-framework/dpf.h>


#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_CanvasViewBroker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        call = false;

        manager = new CanvasManager();
        proxymodel = new CanvasProxyModel();
        selection = new CanvasSelectionModel(proxymodel,nullptr);
        manager->d->canvasModel = proxymodel;
        manager->d->selectionModel = selection;
        broker = new CanvasViewBroker(manager);

        stub.set_lamda(&CanvasViewBroker::getView,[this](CanvasViewBroker*,int){
            __DBG_STUB_INVOKE__
                   CanvasView *view = new CanvasView();
                   QSharedPointer<CanvasView> ptr(view);
                   call = true;
                   return ptr;
        });
    }
    virtual void TearDown() override
    {
        delete selection;
        delete proxymodel;
        delete manager;
        delete broker;
        stub.clear();
    }
    CanvasViewBroker *broker = nullptr;
    CanvasManager * manager = nullptr;
    stub_ext::StubExt stub;
    CanvasProxyModel *proxymodel = nullptr;
    CanvasSelectionModel *selection = nullptr;
    bool call = false;
};

TEST_F(UT_CanvasViewBroker, gridPos)
{
    stub.set_lamda(&CanvasViewPrivate::gridAt,[](CanvasViewPrivate*,const QPoint &point){
        __DBG_STUB_INVOKE__
               return QPoint(1,1);
    });
    QPoint pos = broker->gridPos(1,QPoint(1,1));
    EXPECT_EQ(pos,QPoint(1,1));
    EXPECT_TRUE(call);
}

TEST_F(UT_CanvasViewBroker, gridSize)
{
    QSize size = broker->gridSize((1));
    EXPECT_EQ(size,QSize(1,1));
    EXPECT_TRUE(call);
}
TEST_F(UT_CanvasViewBroker, refresh)
{
    stub.set_lamda(&CanvasView::refresh,[](CanvasView*,bool){
        __DBG_STUB_INVOKE__
    });
    broker->refresh(1);
    EXPECT_TRUE(call);
}

TEST_F(UT_CanvasViewBroker, update)
{
    auto fun_type = static_cast<void(QWidget::*)()>(&QWidget::update);
    stub.set_lamda(fun_type,[](QWidget*){
        __DBG_STUB_INVOKE__
    });
    broker->update(1);
    EXPECT_TRUE(call);
}

TEST_F(UT_CanvasViewBroker, select)
{
    QUrl url1 = QUrl::fromLocalFile("file:/temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file:/temp_url2");
    QList<QUrl> urls{url1,url2};
    EXPECT_NO_FATAL_FAILURE(broker->select(urls));
}

TEST_F(UT_CanvasViewBroker, selectedUrls)
{
    stub.set_lamda(&CanvasSelectionModel::selectedUrls,[](){
        __DBG_STUB_INVOKE__
                QList<QUrl> res;
                res.push_back(QUrl("temp_url"));
                return res;
    });
    QList<QUrl> lists = broker->selectedUrls(1);
    EXPECT_TRUE(call);
    EXPECT_TRUE(lists.isEmpty());
}

TEST_F(UT_CanvasViewBroker, iconRect)
{
    CanvasView view ;
    CanvasItemDelegate item(&view);
    stub.set_lamda(&CanvasView::itemDelegate,[&item](){
        __DBG_STUB_INVOKE__
                return &item;
    });
    broker->iconRect(1,QRect(1,1,2,2));
    EXPECT_TRUE(call);
}
