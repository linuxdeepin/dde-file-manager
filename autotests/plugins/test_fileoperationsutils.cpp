// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationsutils.cpp
 * @brief Unit tests for FileOperationsUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/fileoperationsutils.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperationsUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperationsUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsUtilsTest, isAncestorUrl)
{
    // Test method: bool isAncestorUrl((const QUrl &from, const QUrl &to))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->isAncestorUrl(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileOperationsUtilsTest, parentUrl)
{
    // Test method: QUrl parentUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->parentUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperationsUtilsTest, blockSync)
{
    // Test bool getter: blockSync()
    bool result = obj->blockSync();
    EXPECT_FALSE(result);

}

TEST_F(FileOperationsUtilsTest, canBroadcastPaste)
{
    // Test bool getter: canBroadcastPaste()
    bool result = obj->canBroadcastPaste();
    EXPECT_FALSE(result);

}

TEST_F(FileOperationsUtilsTest, isFileOnDisk)
{
    // Test method: bool isFileOnDisk((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isFileOnDisk(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileOperationsUtilsTest, isFilesSizeOutLimit)
{
    // Test method: bool isFilesSizeOutLimit((const QUrl &url, const qint64 limitSize))
    QUrl _arg0{};
    auto result = obj->isFilesSizeOutLimit(_arg0, 0);
    EXPECT_FALSE(result);

}
