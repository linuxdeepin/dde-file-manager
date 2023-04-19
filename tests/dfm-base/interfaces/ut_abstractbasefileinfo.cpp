// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_ABSTRACTFILEINFO
#define UT_ABSTRACTFILEINFO

#include <dfm-base/interfaces/abstractfileinfo.h>

#include <QDir>
#include <QStandardPaths>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_AbstractFileInfo : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    ~UT_AbstractFileInfo() override;

    AbstractFileInfoPointer info{ nullptr };
};

UT_AbstractFileInfo::~UT_AbstractFileInfo() {

}

TEST_F(UT_AbstractFileInfo, testFileBaseInfo)
{
    auto url = QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    info.reset(new AbstractFileInfo(QUrl(url.toString() + QDir::separator())));
    info.reset(new AbstractFileInfo(url));

    EXPECT_EQ(url, info->fileUrl());
    EXPECT_EQ(false, info->exists());
    EXPECT_EQ(url.path(), info->filePath());
    EXPECT_EQ(url.path(), info->absoluteFilePath());
    EXPECT_NO_FATAL_FAILURE(info->refresh());
    EXPECT_EQ("Desktop", info->fileName());
    EXPECT_EQ("Desktop", info->baseName());
    url.setPath(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/uniontech.txt.txt");
    info.reset(new AbstractFileInfo(url));
    EXPECT_EQ("uniontech.txt", info->baseName());
    EXPECT_EQ("uniontech.txt.txt", info->completeBaseName());
    EXPECT_EQ("txt", info->suffix());
    EXPECT_EQ("txt.txt", info->completeSuffix());
    EXPECT_EQ(QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()).path(), info->path());
    EXPECT_EQ(QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()).path(), info->absolutePath());
    EXPECT_EQ(false, info->isReadable());
    EXPECT_EQ(false, info->isWritable());
    EXPECT_EQ(false, info->isExecutable());
    EXPECT_EQ(false, info->isHidden());
    EXPECT_EQ(false, info->isNativePath());
    EXPECT_EQ(false, info->isFile());
    EXPECT_EQ(false, info->isDir());
    EXPECT_EQ(false, info->isSymLink());
    EXPECT_EQ(false, info->isRoot());
    EXPECT_EQ(false, info->isBundle());
    EXPECT_EQ("", info->symLinkTarget());
    EXPECT_EQ("", info->owner());
    EXPECT_EQ("", info->group());
    EXPECT_EQ("0", QString::number(info->ownerId()));
    EXPECT_EQ("0", QString::number(info->groupId()));
    EXPECT_EQ("0", QString::number(info->countChildFile()));
    EXPECT_EQ("0", QString::number(info->size()));
    EXPECT_EQ("0", QString::number(info->birthTime()));
    EXPECT_EQ("0", QString::number(info->metadataChangeTime()));
    EXPECT_EQ("0", QString::number(info->lastModified()));
    EXPECT_EQ("0", QString::number(info->lastRead()));
    EXPECT_FALSE(info->permission(QFile::ReadOwner));
    EXPECT_FALSE(info->permissions());
}

#endif
