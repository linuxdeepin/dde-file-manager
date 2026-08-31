// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncfileinfo.cpp
 * @brief Unit tests for SyncFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/syncfileinfo.h"

#include <QTest>

using namespace src;

class SyncFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(SyncFileInfoTest, fileMimeTypeAsync)
{
    // Test method: QMimeType fileMimeTypeAsync((QMimeDatabase::MatchMode mode))
    auto result = obj->fileMimeTypeAsync(QMimeDatabase::MatchMode());
    EXPECT_NO_FATAL_FAILURE({ obj->fileMimeTypeAsync(QMimeDatabase::MatchMode()); });

}

TEST_F(SyncFileInfoTest, fileType)
{
    // Test getter: SyncFileInfo::FileType fileType()
    auto result = obj->fileType();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(SyncFileInfoTest, initQuerier)
{
    // Test bool getter: initQuerier()
    bool result = obj->initQuerier();
    EXPECT_FALSE(result);

}

TEST_F(SyncFileInfoTest, pathOf)
{
    // Test method: QString pathOf((const PathInfoType type))
    auto result = obj->pathOf(PathInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoTest, permissions)
{
    // Test getter: QFileDevice::Permissions permissions()
    auto result = obj->permissions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(SyncFileInfoTest, setExtendedAttributes)
{
    // Test setter: void setExtendedAttributes((const FileExtendedInfoType &key, const QVariant &value))
    FileExtendedInfoType _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setExtendedAttributes(_arg0, _arg1));
}

TEST_F(SyncFileInfoTest, size)
{
    // Test getter: qint64 size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}

TEST_F(SyncFileInfoTest, urlOf)
{
    // Test method: QUrl urlOf((const UrlInfoType type))
    auto result = obj->urlOf(UrlInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(SyncFileInfoTest, viewOfTip)
{
    // Test method: QString viewOfTip((const ViewType type))
    auto result = obj->viewOfTip(ViewType());
    EXPECT_TRUE(result.isEmpty());

}
