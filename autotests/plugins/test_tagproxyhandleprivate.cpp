// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagproxyhandleprivate.cpp
 * @brief Unit tests for TagProxyHandlePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "data/tagproxyhandle.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagProxyHandlePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagProxyHandlePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagProxyHandlePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagProxyHandlePrivateTest, TagProxyHandlePrivate)
{
    // Test constructor: TagProxyHandlePrivate((TagProxyHandle *qq, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagProxyHandlePrivateTest, connectToDBus)
{
    // Test method: void connectToDBus(())
    EXPECT_NO_FATAL_FAILURE(obj->connectToDBus());
}

TEST_F(TagProxyHandlePrivateTest, disconnCurrentConnections)
{
    // Test method: void disconnCurrentConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->disconnCurrentConnections());
}

TEST_F(TagProxyHandlePrivateTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}

TEST_F(TagProxyHandlePrivateTest, isDBusRuning)
{
    // Test bool getter: isDBusRuning()
    bool result = obj->isDBusRuning();
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandlePrivateTest, parseDBusVariant)
{
    // Test method: QVariant parseDBusVariant((const QDBusVariant &var))
    QDBusVariant _arg0{};
    auto result = obj->parseDBusVariant(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagProxyHandlePrivateTest, TagProxyHandlePrivate_Destructor)
{
    // Test method:  ~TagProxyHandlePrivate(())
    EXPECT_NO_FATAL_FAILURE({ TagProxyHandlePrivate *tmp = new TagProxyHandlePrivate(); delete tmp; });
}
