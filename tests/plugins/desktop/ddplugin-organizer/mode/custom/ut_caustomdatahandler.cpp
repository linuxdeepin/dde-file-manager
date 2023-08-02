// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/custom/customdatahandler.h"


#include "stubext.h"
#include <gtest/gtest.h>
#include <QSet>
#include <QMimeData>

using namespace testing;
using namespace ddplugin_organizer;

class CustomDataHandlerTest : public Test
{
public:
    virtual void SetUp() override {
        handler = new CustomDataHandler();
    }
    virtual void TearDown() override {
        delete handler;
        stub.clear();
    }

    CustomDataHandler *handler = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomDataHandlerTest, check)
{
    QUrl url("temp_url");
    QUrl url1("temp_url1");
    QSet<QUrl> vaild{url};

    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    ptr->items.push_back(url1);
    handler->collections.insert("temp_str", ptr);
    EXPECT_NO_FATAL_FAILURE(handler->check(vaild));
    EXPECT_FALSE(handler->collections["temp_str"]->items.contains(url1));
}

TEST_F(CustomDataHandlerTest, remove)
{
    QUrl url("temp_url");
    QUrl url1("temp_url1");
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    ptr->items.push_back(url1);
    handler->collections.insert("temp_str", ptr);
    QString res = handler->remove(url);
    EXPECT_EQ(res,"temp_str");

    res = handler->remove(url);
    EXPECT_EQ(res,"");
}

TEST_F(CustomDataHandlerTest, replace)
{
    QUrl url("temp_url");
    QUrl url1("temp_url1");
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    ptr->key = QString("temp_key");
    handler->collections.insert("temp_str", ptr);

    QString res = handler->replace(url,url1);
    EXPECT_EQ(res,"temp_key");

    handler->collections["temp_str"]->items.push_back(url1);
    res = handler->replace(url,url1);
    EXPECT_EQ(res,nullptr);
}

TEST_F(CustomDataHandlerTest, acceptRename)
{
    QUrl url("temp_url");
    QUrl url1("temp_url1");
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    handler->collections.insert("temp_str", ptr);
    EXPECT_TRUE(handler->acceptRename(url,url1));
}

TEST_F(CustomDataHandlerTest, acceptInsert)
{
    QUrl url("temp_url");
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    handler->collections.insert("temp_str", ptr);
    EXPECT_TRUE(handler->acceptInsert(url));
}

TEST_F(CustomDataHandlerTest, acceptReset)
{
    QUrl url("temp_url");
    QUrl url1("temp_url1");
    QList<QUrl> list{url,url1};
    CollectionBaseDataPtr ptr(new CollectionBaseData());
    ptr->items.push_back(url);
    handler->collections.insert("temp_str", ptr);
    QList<QUrl> res =  handler->acceptReset(list);
    EXPECT_TRUE(res.contains(url));
    EXPECT_FALSE(res.contains(url1));
}
TEST_F(CustomDataHandlerTest, test)
{
    EXPECT_EQ(handler->append(QUrl("temp_str")),nullptr);
    EXPECT_EQ(handler->prepend(QUrl("temp_str")),nullptr);
}
