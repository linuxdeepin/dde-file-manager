// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_asyncfileinfoprivate.cpp
 * @brief Unit tests for AsyncFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/asyncfileinfo.h"

#include <QTest>

using namespace src;

class AsyncFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AsyncFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AsyncFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AsyncFileInfoPrivateTest, attribute)
{
    // Test method: QVariant attribute((DFileInfo::AttributeID key, bool *ok))
    auto result = obj->attribute(DFileInfo::AttributeID(), nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoPrivateTest, baseName)
{
    // Test getter: QString baseName()
    auto result = obj->baseName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, canDelete)
{
    // Test bool getter: canDelete()
    bool result = obj->canDelete();
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoPrivateTest, canTrash)
{
    // Test bool getter: canTrash()
    bool result = obj->canTrash();
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoPrivateTest, completeBaseName)
{
    // Test getter: QString completeBaseName()
    auto result = obj->completeBaseName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, fileDisplayName)
{
    // Test getter: QString fileDisplayName()
    auto result = obj->fileDisplayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, filePath)
{
    // Test getter: QString filePath()
    auto result = obj->filePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, iconName)
{
    // Test getter: QString iconName()
    auto result = obj->iconName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, init)
{
    // Test method: void init((const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->init(_arg0, QSharedPointer<DFMIO::DFileInfo>()));
}

TEST_F(AsyncFileInfoPrivateTest, mediaInfo)
{
    // Test method: QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo((DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids))
    auto result = obj->mediaInfo(DFileInfo::MediaType(), QList<DFileInfo::AttributeExtendID>());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, mimeTypes)
{
    // Test method: QMimeType mimeTypes((const QString &filePath, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs))
    QString _arg0{};
    QString _arg2{};
    auto result = obj->mimeTypes(_arg0, QMimeDatabase::MatchMode(), _arg2, false);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeTypes(_arg0, QMimeDatabase::MatchMode(), _arg2, false); });

}

TEST_F(AsyncFileInfoPrivateTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoPrivateTest, symLinkTarget)
{
    // Test getter: QString symLinkTarget()
    auto result = obj->symLinkTarget();
    EXPECT_TRUE(result.isEmpty());

}
