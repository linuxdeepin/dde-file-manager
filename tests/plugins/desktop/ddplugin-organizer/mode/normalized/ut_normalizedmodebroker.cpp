// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/normalizedmodebroker.h"
#include "mode/normalized/normalizedmode_p.h"
#include "broker/collectionviewbroker.h"

#include "delegate/collectionitemdelegate.h"
#include "models/collectionmodel.h"
#include "view/collectionview.h"

#include "collection/collectionholder.h"


#include "view/collectionwidget.h"
#include "view/collectionwidget_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;
using namespace testing;

class TestNormalizedModeBroker :  public Test
{
public:
    void SetUp() override {
        parent = new NormalizedMode();
        broker = new NormalizedModeBroker(parent);
    }

    void TearDown() override {
        delete broker;
        stub.clear();;
    }
    stub_ext::StubExt stub;
    NormalizedMode *parent = nullptr;
    NormalizedModeBroker *broker = nullptr;
};

TEST_F(TestNormalizedModeBroker, refreshModel)
{
    bool call = false;
    stub.set_lamda(&CollectionModel::refresh,
                   [&call](CollectionModel*, const QModelIndex &parent, bool global, int ms, bool file){
        call = true;
    });
    broker->mode->model = new CollectionModel();
    broker->refreshModel(true,1,true);
    EXPECT_TRUE(call);
}

TEST_F(TestNormalizedModeBroker, gridPoint)
{
    typedef bool(*fun_type)(const QUrl &file, QPoint &pos);
    stub.set_lamda((fun_type)&CollectionViewBroker::gridPoint,
                   [](const QUrl &file, QPoint &pos){
        return true;
    });
    CollectionWidget  *widget=new CollectionWidget("uuid",nullptr);
    stub.set_lamda(&CollectionHolder::widget,[&widget](){
        return widget;
    });
    stub.set_lamda(&CollectionHolder::id,[](){
        return "temp_str";
    });
    QUrl url("temp_url");
    QPoint point(1,1);
    CollectionView *view = new CollectionView("uuid",nullptr);

    widget->d->view = view;
    CollectionHolderPointer ptr(new CollectionHolder("uuid",nullptr));

    broker->mode->d->holders.insert("temp_str",ptr);
    EXPECT_EQ(broker->gridPoint(url,&point),"temp_str");
    delete widget;
    delete view;
}

TEST_F(TestNormalizedModeBroker, visualRect)
{
    CollectionWidget  *widget=new CollectionWidget("uuid",nullptr);
    CollectionView *view = new CollectionView("uuid",nullptr);
    widget->d->view = view;
    stub.set_lamda(&CollectionHolder::widget,[&widget](){
        return widget;
    });
    stub.set_lamda(&CollectionViewBroker::visualRect,[](CollectionViewBroker*, const QUrl &file){
        return QRect(1,1,2,2);
    });
    QString str("temp_str");
    QUrl url("temp_url");
    CollectionHolderPointer ptr(new CollectionHolder("uuid",nullptr));

    broker->mode->d->holders.insert("temp_str",ptr);
    QRect res =  broker->visualRect(str,url);
    EXPECT_EQ(res,QRect(1,1,2,2));
}

TEST_F(TestNormalizedModeBroker, view)
{
    CollectionWidget  *widget=new CollectionWidget("uuid",nullptr);
    CollectionView *view = new CollectionView("uuid",nullptr);
    widget->d->view = view;
    stub.set_lamda(&CollectionHolder::widget,[&widget](){
        return widget;
    });
    CollectionHolderPointer ptr(new CollectionHolder("uuid",nullptr));
    broker->mode->d->holders.insert("temp_str",ptr);
    auto res =  broker->view("temp_str");
    EXPECT_EQ(res,view);
}

TEST_F(TestNormalizedModeBroker, iconRect)
{
    CollectionWidget  *widget=new CollectionWidget("uuid",nullptr);
    CollectionView *view = new CollectionView("uuid",nullptr);
    widget->d->view = view;
    stub.set_lamda(&CollectionHolder::widget,[&widget](){
        return widget;
    });
    stub.set_lamda(&CollectionItemDelegate::iconRect,
                   [](CollectionItemDelegate*,const QRect &paintRect){return QRect(1,1,2,2);});
    CollectionHolderPointer ptr(new CollectionHolder("uuid",nullptr));
    broker->mode->d->holders.insert("temp_str",ptr);
    QString url("temp_str");
    QRect res = broker->iconRect(url,QRect(1,1,2,2));
    EXPECT_EQ(res,QRect(1,1,2,2));
}

