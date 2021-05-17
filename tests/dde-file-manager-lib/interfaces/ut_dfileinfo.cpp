/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include "interfaces/dfileinfo.h"
#include "testhelper.h"

#include <QStandardPaths>
#include <QIcon>

namespace {
class TestDFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        m_dirPathStr = TestHelper::createTmpDir();
        m_filePathStr = TestHelper::createTmpFile(".txt");
        m_symlinkPathStr = TestHelper::createTmpSymlinkFile(m_filePathStr);
        m_pDirInfo = new DFileInfo(m_dirPathStr);
        m_pFileInfo = new DFileInfo(QFileInfo(m_filePathStr));
        m_pSymLinkInfo = new DFileInfo(QFileInfo(m_symlinkPathStr));
    }

    void TearDown() override
    {
        if (m_pDirInfo != nullptr) {
            delete m_pDirInfo;
            m_pDirInfo = nullptr;
        }

        if (m_pSymLinkInfo != nullptr) {
            delete m_pSymLinkInfo;
            m_pSymLinkInfo = nullptr;
        }

        if (m_pFileInfo != nullptr) {
            delete m_pFileInfo;
            m_pFileInfo = nullptr;
        }

        TestHelper::deleteTmpFiles({m_symlinkPathStr, m_filePathStr, m_dirPathStr});
    }

public:
    QString m_filePathStr;
    QString m_dirPathStr;
    QString m_symlinkPathStr;

    DFileInfo *m_pDirInfo;
    DFileInfo *m_pFileInfo;
    DFileInfo *m_pSymLinkInfo;
};
} // namespace

TEST_F(TestDFileInfo, test_fileinfo_property)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->isFile());
    EXPECT_FALSE(m_pFileInfo->isDir());
    EXPECT_FALSE(m_pFileInfo->makeAbsolute());
    EXPECT_TRUE(m_pFileInfo->exists());
    EXPECT_TRUE(m_pFileInfo->canRename());
    EXPECT_TRUE(m_pFileInfo->isWritable());
    EXPECT_TRUE(m_pFileInfo->isReadable());
    EXPECT_FALSE(m_pFileInfo->canRedirectionFileUrl());
    EXPECT_TRUE(m_pFileInfo->canIteratorDir());
    EXPECT_FALSE(m_pFileInfo->canDrop());
    EXPECT_FALSE(m_pFileInfo->canShare());
    EXPECT_FALSE(m_pFileInfo->canFetch());
    EXPECT_FALSE(m_pFileInfo->canTag());
    EXPECT_FALSE(m_pFileInfo->isVirtualEntry());
    EXPECT_NE(m_pFileInfo->permissions(), 0);
    EXPECT_NE(m_pFileInfo->permission(m_pFileInfo->permissions()), 0);
    EXPECT_EQ(m_pFileInfo->fileItemDisableFlags(), Qt::ItemFlag());
    EXPECT_EQ(m_pFileInfo->extraProperties().count(), 0);
    EXPECT_NE(m_pFileInfo->userColumnRoles().count(), 0);
    EXPECT_TRUE(m_pFileInfo->supportedDragActions() & Qt::CopyAction);
    EXPECT_FALSE(m_pFileInfo->isExecutable());
    EXPECT_FALSE(m_pFileInfo->isHidden());
    EXPECT_FALSE(m_pFileInfo->isRelative());
    EXPECT_TRUE(m_pFileInfo->isAbsolute());
    EXPECT_FALSE(m_pFileInfo->isShared());
    EXPECT_FALSE(m_pFileInfo->isWritableShared());
    EXPECT_FALSE(m_pFileInfo->isAllowGuestShared());
    EXPECT_FALSE(m_pFileInfo->isSymLink());
}

TEST_F(TestDFileInfo, test_owner)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_EQ(m_pFileInfo->owner(), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().split("/").last());
    EXPECT_NE(m_pFileInfo->ownerId(), static_cast<uint>(-2));
}

TEST_F(TestDFileInfo, test_group)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_EQ(m_pFileInfo->group(), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().split("/").last());
    EXPECT_NE(m_pFileInfo->groupId(), static_cast<uint>(-2));
}

TEST_F(TestDFileInfo, test_time)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->created().isValid());
    EXPECT_TRUE(m_pFileInfo->lastModified().isValid());
    EXPECT_TRUE(m_pFileInfo->lastRead().isValid());
}

TEST_F(TestDFileInfo, test_size)
{
    ASSERT_NE(m_pFileInfo, nullptr);
    ASSERT_NE(m_pDirInfo, nullptr);

    EXPECT_EQ(m_pFileInfo->size(), 0);
    EXPECT_EQ(m_pFileInfo->sizeDisplayName(), QString("0 B"));
    EXPECT_EQ(m_pDirInfo->sizeDisplayName(), QStringLiteral("-"));
}

TEST_F(TestDFileInfo, test_icon)
{
    // 阻塞CI
    // ASSERT_NE(m_pFileInfo, nullptr);
    // ASSERT_NE(m_pDirInfo, nullptr);
    // ASSERT_NE(m_pSymLinkInfo, nullptr);

    // TestHelper::runInLoop([&]{
    //     m_pFileInfo->fileIcon();
    // }, 1000);

    // EXPECT_FALSE(m_pFileInfo->fileIcon().isNull());
//    EXPECT_FALSE(m_pSymLinkInfo->fileIcon().isNull());
    // EXPECT_EQ(m_pFileInfo->iconName(), QString("text-plain"));
    // EXPECT_EQ(m_pDirInfo->iconName(), QString("inode-directory"));
}

TEST_F(TestDFileInfo, test_getExtraPropertise)
{
    // 阻塞CI
    // ASSERT_NE(m_pFileInfo, nullptr);

    // TestHelper::runInLoop([&]{
    //     m_pFileInfo->extraProperties();
    // }, 800);

    // EXPECT_TRUE(m_pFileInfo->extraProperties().isEmpty());
}

TEST_F(TestDFileInfo, test_getInode)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_NE(m_pFileInfo->inode(), 0);
}

TEST_F(TestDFileInfo, test_toQFileInfo)
{
    ASSERT_NE(m_pFileInfo, nullptr);
    ASSERT_NE(m_pDirInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->toQFileInfo().isFile());
    EXPECT_TRUE(m_pDirInfo->toQFileInfo().isDir());
}

TEST_F(TestDFileInfo, test_createIODevice)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    QIODevice *device = m_pFileInfo->createIODevice();
    EXPECT_NE(device, nullptr);

    FreePointer(device);
}

TEST_F(TestDFileInfo, test_active)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    m_pFileInfo->makeToActive();
    EXPECT_TRUE(m_pFileInfo->isActive());

    m_pFileInfo->makeToInactive();
    EXPECT_FALSE(m_pFileInfo->isActive());
}

TEST_F(TestDFileInfo, test_goToUrlWhenDeleted)
{
    ASSERT_NE(m_pFileInfo, nullptr);
    ASSERT_NE(m_pDirInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->goToUrlWhenDeleted().isValid());
    EXPECT_TRUE(m_pDirInfo->goToUrlWhenDeleted().isValid());
}

TEST_F(TestDFileInfo, test_subtitleForEmptyFloder)
{
    ASSERT_NE(m_pDirInfo, nullptr);

    EXPECT_EQ(m_pDirInfo->subtitleForEmptyFloder(), QObject::tr("Folder is empty"));
}

TEST_F(TestDFileInfo, test_mimeType)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->mimeType().isValid());
}

TEST_F(TestDFileInfo, test_symlink)
{
    ASSERT_NE(m_pFileInfo, nullptr);
    ASSERT_NE(m_pSymLinkInfo, nullptr);

    EXPECT_FALSE(m_pSymLinkInfo->symlinkTargetPath().isEmpty());
    EXPECT_TRUE(m_pFileInfo->symlinkTargetPath().isEmpty());

    EXPECT_EQ(m_pSymLinkInfo->symLinkTarget().path(), m_filePathStr);
    EXPECT_EQ(m_pFileInfo->symLinkTarget(), DUrl());
}

TEST_F(TestDFileInfo, test_path)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_EQ(m_pFileInfo->path(), m_filePathStr.left(m_filePathStr.lastIndexOf("/")));
    EXPECT_EQ(m_pFileInfo->filePath(), m_filePathStr);
    EXPECT_EQ(m_pFileInfo->absolutePath(), m_filePathStr.left(m_filePathStr.lastIndexOf("/")));
    EXPECT_EQ(m_pFileInfo->absoluteFilePath(), m_filePathStr);
    EXPECT_EQ(m_pFileInfo->fileName(), m_filePathStr.split("/").last());
}

TEST_F(TestDFileInfo, test_additionalIcon)
{
    ASSERT_NE(m_pFileInfo, nullptr);
    ASSERT_NE(m_pSymLinkInfo, nullptr);

    EXPECT_FALSE(m_pSymLinkInfo->additionalIcon().isEmpty());
    EXPECT_TRUE(m_pFileInfo->additionalIcon().isEmpty());
}

TEST_F(TestDFileInfo, test_fileIsWritable)
{
    ASSERT_NE(m_pFileInfo, nullptr);

    EXPECT_TRUE(m_pFileInfo->fileIsWritable(m_pFileInfo->path(), 0));
}

TEST_F(TestDFileInfo, test_dirFilesCount)
{
    ASSERT_NE(m_pDirInfo, nullptr);

    EXPECT_EQ(m_pDirInfo->filesCount(), 0);
}

TEST_F(TestDFileInfo, test_static_exist)
{
    EXPECT_TRUE(DFileInfo::exists(DUrl::fromLocalFile(m_filePathStr)));
}

TEST_F(TestDFileInfo, test_static_mimeType)
{
    EXPECT_TRUE(DFileInfo::mimeType(m_filePathStr, QMimeDatabase::MatchDefault, QString(), true).isValid());
    EXPECT_TRUE(DFileInfo::mimeType(m_filePathStr).isValid());
}
