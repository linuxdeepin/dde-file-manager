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

#include "models/networkfileinfo.h"
#include "views/dfileview.h"

#include <gtest/gtest.h>
#include <QTimer>

namespace {
class TestNetworkFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestNetworkFileInfo";
        info = new NetworkFileInfo(DUrl("file:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestNetworkFileInfo";
        QEventLoop loop;
        QTimer::singleShot(200, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete info;
    }

public:
    NetworkFileInfo *info;
};
} // namespace

TEST_F(TestNetworkFileInfo, filePath)
{
    EXPECT_STREQ("", info->filePath().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, absoluteFilePath)
{
    EXPECT_STREQ("", info->absoluteFilePath().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, isFileExists)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(TestNetworkFileInfo, isFileReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestNetworkFileInfo, isFileWritable)
{
    EXPECT_TRUE(info->isWritable());
}

TEST_F(TestNetworkFileInfo, isFileVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestNetworkFileInfo, canFileDrop)
{
    EXPECT_TRUE(info->canDrop());
}

TEST_F(TestNetworkFileInfo, canFileRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestNetworkFileInfo, canFileIterator)
{
    EXPECT_FALSE(info->canIteratorDir());
}

TEST_F(TestNetworkFileInfo, fileIsDir)
{
    EXPECT_TRUE(info->isDir());
}

TEST_F(TestNetworkFileInfo, parentUrl)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, fileDisplayName)
{
    EXPECT_STREQ("", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, filesCount)
{
    EXPECT_EQ(-1, info->filesCount());
}

TEST_F(TestNetworkFileInfo, iconName)
{
    EXPECT_STREQ("", info->iconName().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, canRedirectUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestNetworkFileInfo, redirectedUrl)
{
    EXPECT_STREQ("", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, tstNetworkNode)
{
    info->setNetworkNode(NetworkNode());
    auto n = info->networkNode();
    EXPECT_TRUE(n.url().isNull());
}

TEST_F(TestNetworkFileInfo, tstMenuActionList)
{
    auto type = DAbstractFileInfo::MenuType::SpaceArea;
    EXPECT_TRUE(info->menuActionList(type).count() == 0);
    type = DAbstractFileInfo::MenuType::SingleFile;
    EXPECT_TRUE(info->menuActionList(type).count() == 2);
    type = DAbstractFileInfo::MenuType::MultiFiles;
    EXPECT_TRUE(info->menuActionList(type).count() == 0);
}

TEST_F(TestNetworkFileInfo, tstSupportSelectionModes)
{
    EXPECT_TRUE(info->supportSelectionModes().count() == 1);
}

TEST_F(TestNetworkFileInfo, tstFileItemDisableFlags)
{
    EXPECT_TRUE(info->fileItemDisableFlags().testFlag(Qt::ItemIsEditable));
}

TEST_F(TestNetworkFileInfo, tstSupportViewMode)
{
    EXPECT_TRUE(info->supportViewMode() == DFileView::IconMode);
}
