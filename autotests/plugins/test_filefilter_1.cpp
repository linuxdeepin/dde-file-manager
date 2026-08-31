// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filefilter_1.cpp
 * @brief Unit tests for FileFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/filefilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileFilterTest, FileFilter)
{
    // Test constructor: FileFilter(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileFilterTest, fileCreatedFilter)
{
    // Test method: bool fileCreatedFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileCreatedFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileFilterTest, fileRenameFilter)
{
    // Test method: bool fileRenameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->fileRenameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileFilterTest, fileTraversalFilter)
{
    // Test method: bool fileTraversalFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->fileTraversalFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileFilterTest, fileUpdatedFilter)
{
    // Test method: bool fileUpdatedFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileUpdatedFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileFilterTest, FileFilter_Destructor)
{
    // Test method:  ~FileFilter(())
    EXPECT_NO_FATAL_FAILURE({ FileFilter *tmp = new FileFilter(); delete tmp; });
}
