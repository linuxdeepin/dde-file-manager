// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcachecontroller.cpp
 * @brief Unit tests for FileTagCacheController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCacheControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCacheController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCacheController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCacheControllerTest, findChildren)
{
    // Test method: QHash<QString, QStringList> findChildren((const QString &parentPath))
    QString _arg0{};
    auto result = obj->findChildren(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheControllerTest, getTagsByFiles)
{
    // Test method: QStringList getTagsByFiles((const QStringList &paths))
    QStringList _arg0{};
    auto result = obj->getTagsByFiles(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
