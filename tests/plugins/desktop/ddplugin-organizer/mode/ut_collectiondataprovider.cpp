// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include"mode/collectiondataprovider.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

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

class UT_CollectionDataProvider : public testing::Test
{
protected:
    virtual void SetUp() override {
        prov = new TestProvider;

    }
    virtual void TearDown() override {
        delete prov;
        prov = nullptr;
        stub.clear();
    }

    TestProvider *prov = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionDataProvider, key)
{
    QUrl url("temp");

    QString res = prov->key(url);
    EXPECT_EQ("",res);
    prov->keys();
}

TEST_F(UT_CollectionDataProvider, name)
{
    CollectionBaseDataPtr data(new CollectionBaseData);
    data.value->name = "temp";
    prov->collections["window"] = data;
    const QString str = "window";
    QUrl url("temp");
    prov->items(str);
    prov->contains(str,url);
    QString res = prov->name(str);
    EXPECT_EQ(res,"temp");

}
TEST_F(UT_CollectionDataProvider, moveUrls)
{
    QUrl url("temp");
    CollectionBaseDataPtr data(new CollectionBaseData);
    data.value->items.push_back(url);

    prov->collections["window"] = data;

    QList list{url};
    QString targetKey = "window";
    int targetIndex = 1;

    typedef QString(*fun_type)(const QUrl&);
    stub.set_lamda((fun_type)(&TestProvider::key),[](){
        __DBG_STUB_INVOKE__
        return "window";
    });
    bool call = false;
    QObject::connect(prov,&TestProvider::itemsChanged,[&call](QString){
        call = true;
    });

    prov->moveUrls(list,targetKey, targetIndex);
    EXPECT_TRUE(call);

    targetKey = "no window";
    call = false;
    prov->moveUrls(list,targetKey, targetIndex);
    EXPECT_TRUE(call);
}

TEST_F(UT_CollectionDataProvider, addPreItems)
{
    QUrl url("temp");
    QList list{url};
    QString targetKey = "window";
    int targetIndex = 1;
    prov->addPreItems(targetKey,list,targetIndex);
    prov->addPreItems(targetKey,list,targetIndex);

    auto it = prov->preCollectionItems.find(targetKey);
    QUrl testUrl = (*it).second.first();
    EXPECT_EQ(testUrl,url);
}

TEST_F(UT_CollectionDataProvider, takePreItem)
{
    QUrl url("temp");
    QList list{url};
    QString targetKey = "window";

    prov->preCollectionItems["window"] = QPair<int,QList<QUrl>>(1,list);

    QString key;
    int index;
    EXPECT_TRUE(prov->takePreItem(url,key,index));
    EXPECT_EQ(key,"window");
    EXPECT_EQ(index,1);

    prov->preCollectionItems["window"].second.push_back(url);
    EXPECT_TRUE(prov->checkPreItem(url,key,index));
    EXPECT_EQ(key,"window");
    EXPECT_EQ(index,0);
}


