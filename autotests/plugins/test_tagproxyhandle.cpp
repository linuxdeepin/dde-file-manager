// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagproxyhandle.cpp
 * @brief Unit tests for TagProxyHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "data/tagproxyhandle.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagProxyHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagProxyHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagProxyHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagProxyHandleTest, clearAllTrashTags)
{
    // Test bool getter: clearAllTrashTags()
    bool result = obj->clearAllTrashTags();
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, deleteFileTags)
{
    // Test method: bool deleteFileTags((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->deleteFileTags(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, deleteFiles)
{
    // Test method: bool deleteFiles((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->deleteFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, deleteTags)
{
    // Test method: bool deleteTags((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->deleteTags(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, removeTrashFileTags)
{
    // Test method: bool removeTrashFileTags((const QString &originalPath, qint64 inode))
    QString _arg0{};
    auto result = obj->removeTrashFileTags(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, saveTrashFileTags)
{
    // Test method: bool saveTrashFileTags((const QString &originalPath, qint64 inode, const QStringList &tags))
    QString _arg0{};
    QStringList _arg2{};
    auto result = obj->saveTrashFileTags(_arg0, 0, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, instance)
{
    // Test getter: TagProxyHandle instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
