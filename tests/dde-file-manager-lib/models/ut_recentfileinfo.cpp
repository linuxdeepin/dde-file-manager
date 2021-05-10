/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "models/recentfileinfo.h"
#include "dfmglobal.h"
#include "dfilesystemmodel.h"

#include <gtest/gtest.h>
#include <QTimer>
#include <QProcess>

namespace {
class TestRecentFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestRecentFileInfo";
        info = new RecentFileInfo(DUrl("recent:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestRecentFileInfo";
        QEventLoop loop;
        QTimer::singleShot(200, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete info;
    }

public:
    RecentFileInfo *info;
};
} // namespace

TEST_F(TestRecentFileInfo, makeAbsolute)
{
    EXPECT_TRUE(info->makeAbsolute());
}

TEST_F(TestRecentFileInfo, fileExist)
{
    EXPECT_FALSE(info->exists());
}

TEST_F(TestRecentFileInfo, fileIsDir)
{
    EXPECT_FALSE(info->isDir());
}

TEST_F(TestRecentFileInfo, fileIsReadable)
{
    EXPECT_FALSE(info->isReadable());
}

TEST_F(TestRecentFileInfo, fileIsWritable)
{
    EXPECT_FALSE(info->isWritable());
}

TEST_F(TestRecentFileInfo, fileCanIterator)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestRecentFileInfo, fileCanDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestRecentFileInfo, fileCanRedirectUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestRecentFileInfo, fileCanRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestRecentFileInfo, redirectedUrl)
{
    EXPECT_STREQ("/test.file", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, getUrlByNewFileName)
{
    EXPECT_STREQ("//", info->getUrlByNewFileName("").path().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, toLocalFile)
{
    EXPECT_STREQ("/test.file", info->toLocalFile().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, subtitleForEmptyFolder)
{
    EXPECT_STREQ("Folder is empty", info->subtitleForEmptyFloder().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, gotoUrlWhenDeleted) {
    // 这个用例意义不大
    QProcess process;
    process.start("whoami", QStringList());
    process.waitForFinished();
    const QString &user_name = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

    EXPECT_STREQ(QString("/home/%1").arg(user_name).toStdString().c_str(), info->goToUrlWhenDeleted().path().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, fileDisplayName)
{
    EXPECT_STREQ("test.file", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, mimeTypeDisplayName)
{
    EXPECT_STREQ("Unknown (application/octet-stream)", info->mimeTypeDisplayName().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, isVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestRecentFileInfo, increaseCovrage)
{
    QDateTime curr = QDateTime::currentDateTime();
    info->updateReadTime(curr);
    EXPECT_STREQ(curr.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str(),
                 info->getReadTime().toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
}


//TEST_F(TestRecentFileInfo, tstPermissions)
//{
//    EXPECT_TRUE(info->permissions() == (QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther));
//    RecentFileInfo f(DUrl("file:///usr/bin/dde-file-manager"));
//    EXPECT_FALSE(f.permissions() == (QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther));
//}
#ifndef __arm__
TEST_F(TestRecentFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(info->menuActionList(DAbstractFileInfo::MenuType::SpaceArea).count() == 3);
    EXPECT_TRUE(info->menuActionList(DAbstractFileInfo::MenuType::SingleFile).count() > 3);
}
#endif
TEST_F(TestRecentFileInfo, tstDisableMenuActionList)
{
    EXPECT_FALSE(info->disableMenuActionList().count() == 0);
    RecentFileInfo f(DUrl("recent:///"));
    EXPECT_TRUE(f.disableMenuActionList().count() == 0);
}

TEST_F(TestRecentFileInfo, tstUserColumnRoles)
{
    EXPECT_TRUE(info->userColumnRoles().count() == 5);
}

TEST_F(TestRecentFileInfo, tstUserColumnData)
{
    EXPECT_TRUE(info->userColumnData(DFileSystemModel::FileLastReadRole).toString().isEmpty());
    EXPECT_FALSE(info->userColumnData(DFileSystemModel::FileUserRole + 1).toString().isEmpty());
    EXPECT_TRUE(info->userColumnData(DFileSystemModel::FileUserRole + 2).toString().isEmpty());
}

TEST_F(TestRecentFileInfo, tstUserColumnDisplayName)
{
    EXPECT_TRUE(!info->userColumnDisplayName(DFileSystemModel::FileUserRole + 1).toString().isEmpty());
    EXPECT_TRUE(info->userColumnDisplayName(DFileSystemModel::FileUserRole + 2).toString().isEmpty());
}

TEST_F(TestRecentFileInfo, tstMenuActionByColumnRole)
{
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 1) == MenuAction::AbsolutePath);
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 2) != MenuAction::AbsolutePath);
}

TEST_F(TestRecentFileInfo, tstUserColumnWidth)
{
    QFont f;
    QFontMetrics fm(f);
    EXPECT_TRUE(-1 == info->userColumnWidth(DFileSystemModel::FileNameRole, fm));
    EXPECT_TRUE(80 == info->userColumnWidth(DFileSystemModel::FileSizeRole, fm));
}

TEST_F(TestRecentFileInfo, tstMimeDataUrl)
{
    EXPECT_TRUE(info->mimeDataUrl().isValid());
}

TEST_F(TestRecentFileInfo, tstParentUrl)
{
    EXPECT_TRUE(info->parentUrl() == DUrl(RECENT_ROOT));
}

TEST_F(TestRecentFileInfo, tstCompareFunByColumn)
{
    info->updateInfo();
    EXPECT_TRUE(info->compareFunByColumn(DFileSystemModel::FileLastReadRole));
    EXPECT_TRUE(info->compareFunByColumn(DFileSystemModel::FileUserRole + 1));
    EXPECT_FALSE(info->compareFunByColumn(DFileSystemModel::FileUserRole + 2));
}

TEST_F(TestRecentFileInfo, tstReadDateTime)
{
    QDateTime dt = QDateTime::currentDateTime();
    info->setReadDateTime(dt.toString("yyyy-MM-dd hh:mm:ss"));
    EXPECT_FALSE(dt == info->readDateTime());
}
