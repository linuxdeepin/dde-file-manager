// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncfileinfoprivate.cpp
 * @brief Unit tests for SyncFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/syncfileinfo.h"

#include <QTest>

using namespace src;

class SyncFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncFileInfoPrivateTest, attribute)
{
    // Test method: QVariant attribute((DFileInfo::AttributeID key, bool *ok))
    auto result = obj->attribute(DFileInfo::AttributeID(), nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SyncFileInfoPrivateTest, baseName)
{
    // Test getter: QString baseName()
    auto result = obj->baseName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, canDelete)
{
    // Test bool getter: canDelete()
    bool result = obj->canDelete();
    EXPECT_FALSE(result);

}

TEST_F(SyncFileInfoPrivateTest, canFetch)
{
    // Test bool getter: canFetch()
    bool result = obj->canFetch();
    EXPECT_FALSE(result);

}

TEST_F(SyncFileInfoPrivateTest, canRename)
{
    // Test bool getter: canRename()
    bool result = obj->canRename();
    EXPECT_FALSE(result);

}

TEST_F(SyncFileInfoPrivateTest, completeBaseName)
{
    // Test getter: QString completeBaseName()
    auto result = obj->completeBaseName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, completeSuffix)
{
    // Test getter: QString completeSuffix()
    auto result = obj->completeSuffix();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, fileDisplayName)
{
    // Test getter: QString fileDisplayName()
    auto result = obj->fileDisplayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, filePath)
{
    // Test getter: QString filePath()
    auto result = obj->filePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, iconName)
{
    // Test getter: QString iconName()
    auto result = obj->iconName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, init)
{
    // Test method: void init((const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->init(_arg0, QSharedPointer<DFMIO::DFileInfo>()));
}

TEST_F(SyncFileInfoPrivateTest, mediaInfo)
{
    // Test method: QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo((DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids))
    auto result = obj->mediaInfo(DFileInfo::MediaType(), QList<DFileInfo::AttributeExtendID>());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, mimeTypeName)
{
    // Test getter: QString mimeTypeName()
    auto result = obj->mimeTypeName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncFileInfoPrivateTest, symLinkTarget)
{
    // Test getter: QString symLinkTarget()
    auto result = obj->symLinkTarget();
    EXPECT_TRUE(result.isEmpty());

}
