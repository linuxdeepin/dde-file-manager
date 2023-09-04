// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include "broker/collectionviewbroker.h"
#include "view/collectionview_p.h"

#include "models/collectionmodel.h"

using namespace ddplugin_organizer;

class test_CollectionDataProvider : public CollectionDataProvider
{
protected:
    virtual QString replace(const QUrl &oldUrl, const QUrl &newUrl) { return "";}
    virtual QString append(const QUrl &) { return "";}
    virtual QString prepend(const QUrl &) {return "";}
    virtual void insert(const QUrl &, const QString &, const int) {}
    virtual QString remove(const QUrl &) {return "";}
    virtual QString change(const QUrl &) {return "";}

};

class UT_CollectionViewBroker : public testing::Test
{
protected:
    virtual void SetUp() override {
        v = new CollectionView("uuid",nullptr);
        broker = new CollectionViewBroker();
        test_ptr = new test_CollectionDataProvider();

        QPointer<CollectionDataProvider> ptr(test_ptr);
        v->d->provider = ptr;
        typedef QList<QUrl>(*fun_type)(const QString&);
        stub.set_lamda((fun_type)(&CollectionDataProvider::items),[this](const QString&){
            QList<QUrl> res ;
            res.push_back(QUrl("temp_url"));
            return res;
        });
        v->d->id = QString("temp_id");
    }
    virtual void TearDown() override {
        delete test_ptr;
        delete broker;
        delete v;
        stub.clear();
    }

    CollectionViewBroker *broker = nullptr;
    CollectionView *v = nullptr;
    test_CollectionDataProvider *test_ptr = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionViewBroker, setView)
{
    broker->setView(v);
    EXPECT_EQ(v,broker->view);
    EXPECT_EQ(broker->parent(),v);
}

TEST_F(UT_CollectionViewBroker, gridPoint)
{
    QUrl file("temp_url");
    QPoint pos(1,1);
    broker->setView(v);

    EXPECT_TRUE(broker->gridPoint(file,pos));
    EXPECT_EQ(pos,QPoint(0,0));

}

TEST_F(UT_CollectionViewBroker, visualRect)
{
    QUrl file("temp_url");
    broker->setView(v);
    stub.set_lamda(&CollectionViewPrivate::visualRect,[](CollectionViewPrivate*, const QPoint &pos){return QRect(1,1,2,2);});

    QRect res = broker->visualRect(file);
    EXPECT_EQ(res,QRect(1,1,2,2));

}
