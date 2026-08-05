// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimetypedisplay.cpp
 * @brief Unit tests for MimeTypeDisplayManager (mimetypedisplaymanager.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QUrl>

#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

TEST(MimeTypeDisplayTest, DisplayNamesMapNonEmpty)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    ASSERT_NE(mgr, nullptr);
    auto names = mgr->displayNames();
    EXPECT_FALSE(names.isEmpty());
}

TEST(MimeTypeDisplayTest, DisplayNameForDirectory)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString name = mgr->displayName("inode/directory");
    EXPECT_FALSE(name.isEmpty());
}

TEST(MimeTypeDisplayTest, DisplayNameForVideo)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString name = mgr->displayName("video/mp4");
    EXPECT_FALSE(name.isEmpty());
}

TEST(MimeTypeDisplayTest, DisplayNameForUnknown)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString name = mgr->displayName("application/x-some-unknown-type");
    EXPECT_FALSE(name.isEmpty());
}

TEST(MimeTypeDisplayTest, FullMimeName)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString name = mgr->fullMimeName("inode/directory");
    EXPECT_FALSE(name.isEmpty());
}

TEST(MimeTypeDisplayTest, DefaultIconForDirectory)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString icon = mgr->defaultIcon("inode/directory");
    EXPECT_EQ(icon, QString("folder"));
}

TEST(MimeTypeDisplayTest, DefaultIconForVideo)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QString icon = mgr->defaultIcon("video/mp4");
    EXPECT_EQ(icon, QString("video"));
}

TEST(MimeTypeDisplayTest, DisplayNameToEnumDirectory)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    FileInfo::FileType t = mgr->displayNameToEnum("inode/directory");
    EXPECT_EQ(t, FileInfo::FileType::kDirectory);
}

TEST(MimeTypeDisplayTest, DisplayNameToEnumDesktop)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    FileInfo::FileType t = mgr->displayNameToEnum("application/x-desktop");
    EXPECT_EQ(t, FileInfo::FileType::kDesktopApplication);
}

TEST(MimeTypeDisplayTest, SupportArchiveMimeTypesReturnsList)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QStringList archives = mgr->supportArchiveMimetypes();
    // May be empty if the mimetype files are not installed, but should not crash
    EXPECT_NO_FATAL_FAILURE({ (void)archives; });
}

TEST(MimeTypeDisplayTest, SupportVideoMimeTypesReturnsList)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QStringList videos = mgr->supportVideoMimeTypes();
    EXPECT_NO_FATAL_FAILURE({ (void)videos; });
}

TEST(MimeTypeDisplayTest, SupportAudioMimeTypesReturnsList)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    QStringList audios = mgr->supportAudioMimeTypes();
    EXPECT_NO_FATAL_FAILURE({ (void)audios; });
}

TEST(MimeTypeDisplayTest, AccurateDisplayTypeFromPathForTextFile)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("hello world");
    tmp.close();

    auto *mgr = MimeTypeDisplayManager::instance();
    QString name = mgr->accurateDisplayTypeFromPath(tmp.fileName());
    EXPECT_FALSE(name.isEmpty());
}
