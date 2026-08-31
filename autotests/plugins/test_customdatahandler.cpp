// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdatahandler.cpp
 * @brief Unit tests for CustomDataHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/custom/customdatahandler.h"

#include <QTest>

using namespace ddplugin_organizer;

class CustomDataHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomDataHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomDataHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomDataHandlerTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomDataHandlerTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CustomDataHandlerTest, acceptReset)
{
    // Test method: QList<QUrl> acceptReset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->acceptReset(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, append)
{
    // Test method: QString append((const QUrl &))
    QUrl _arg0{};
    auto result = obj->append(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, change)
{
    // Test method: QString change((const QUrl &))
    QUrl _arg0{};
    auto result = obj->change(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, check)
{
    // Test method: void check((const QSet<QUrl> &vaild))
    QSet<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->check(_arg0));
}

TEST_F(CustomDataHandlerTest, insert)
{
    // Test method: void insert((const QUrl &url, const QString &key, const int index))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insert(_arg0, _arg1, 0));
}

TEST_F(CustomDataHandlerTest, prepend)
{
    // Test method: QString prepend((const QUrl &))
    QUrl _arg0{};
    auto result = obj->prepend(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, remove)
{
    // Test method: QString remove((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->remove(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, removeBaseData)
{
    // Test method: void removeBaseData((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeBaseData(_arg0));
}

TEST_F(CustomDataHandlerTest, reset)
{
    // Test method: bool reset((const QList<CollectionBaseDataPtr> &datas))
    QList<CollectionBaseDataPtr> _arg0{};
    auto result = obj->reset(_arg0);
    EXPECT_FALSE(result);

}
