// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localdiriteratorprivate.cpp
 * @brief Unit tests for LocalDirIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localdiriterator.h"

#include <QTest>

using namespace src;

class LocalDirIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalDirIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalDirIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalDirIteratorPrivateTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QSharedPointer<DFileInfo> dfmInfo))
    auto result = obj->fileInfo(QSharedPointer<DFileInfo>());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(LocalDirIteratorPrivateTest, fileInfos)
{
    // Test getter: QList<FileInfoPointer> fileInfos()
    auto result = obj->fileInfos();
    EXPECT_TRUE(result.isEmpty());

}
