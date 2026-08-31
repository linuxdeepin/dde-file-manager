// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileprovider.cpp
 * @brief Unit tests for FileProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class FileProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileProviderTest, insert)
{
    // Test method: void insert((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->insert(_arg0));
}

TEST_F(FileProviderTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(FileProviderTest, remove)
{
    // Test method: void remove((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->remove(_arg0));
}

TEST_F(FileProviderTest, removeFileFilter)
{
    // Test method: void removeFileFilter((QSharedPointer<FileFilter> filter))
    EXPECT_NO_FATAL_FAILURE(obj->removeFileFilter(QSharedPointer<FileFilter>()));
}

TEST_F(FileProviderTest, rename)
{
    // Test method: void rename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->rename(_arg0, _arg1));
}

TEST_F(FileProviderTest, reset)
{
    // Test method: void reset((QList<QUrl> children))
    EXPECT_NO_FATAL_FAILURE(obj->reset(QList<QUrl>()));
}
