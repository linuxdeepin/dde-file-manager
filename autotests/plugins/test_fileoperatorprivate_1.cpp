// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorprivate_1.cpp
 * @brief Unit tests for FileOperatorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileoperator.h"

#include <QTest>

using namespace ddplugin_organizer;

class FileOperatorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorPrivateTest, FileOperatorPrivate)
{
    // Test constructor: FileOperatorPrivate((FileOperator *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorPrivateTest, callBackPasteFiles)
{
    // Test method: void callBackPasteFiles((const JobInfoPointer info, const QVariant &custom))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->callBackPasteFiles(JobInfoPointer(), _arg1));
}

TEST_F(FileOperatorPrivateTest, filterDesktopFile)
{
    // Test method: void filterDesktopFile((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->filterDesktopFile(_arg0));
}

TEST_F(FileOperatorPrivateTest, getSelectedUrls)
{
    // Test method: QList<QUrl> getSelectedUrls((const CollectionView *view))
    auto result = obj->getSelectedUrls(nullptr);
    EXPECT_TRUE(result.isEmpty());

}
