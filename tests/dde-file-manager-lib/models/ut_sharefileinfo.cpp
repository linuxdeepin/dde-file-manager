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

#include "models/sharefileinfo.h"
#include "dfilesystemmodel.h"

#include <gtest/gtest.h>
#include <QIcon>

namespace {
class TestShareFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestShareFileInfo";
        info = new ShareFileInfo(DUrl());
    }

    void TearDown() override
    {
        std::cout << "end TestShareFileInfo";
        delete info;
    }

public:
    ShareFileInfo *info;
};
} // namespace

TEST_F(TestShareFileInfo, tstBasicProperties)
{
    EXPECT_TRUE(info->exists());
    EXPECT_TRUE(info->isDir());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isReadable());
    EXPECT_FALSE(info->isWritable());
    EXPECT_TRUE("" == info->fileDisplayName());
    EXPECT_FALSE(info->canIteratorDir());
    EXPECT_TRUE(info->makeAbsolute());
    EXPECT_FALSE(info->mimeDataUrl().isValid());
    EXPECT_TRUE(info->isShared());
    EXPECT_FALSE(info->canTag());
    EXPECT_FALSE(info->parentUrl().isValid());
    EXPECT_FALSE(info->isVirtualEntry());
    ShareFileInfo i(DUrl(USERSHARE_ROOT));
    EXPECT_FALSE(info->isVirtualEntry());
    EXPECT_FALSE(info->canDrop());
    EXPECT_TRUE((Qt::ItemIsDragEnabled | Qt::ItemIsEditable) == info->fileItemDisableFlags());
    EXPECT_TRUE(info->additionalIcon().count() > 0);
    EXPECT_FALSE(info->canRedirectionFileUrl());
    EXPECT_FALSE(info->redirectedFileUrl().isValid());
}

TEST_F(TestShareFileInfo, tstFuncsWithRole)
{
    EXPECT_FALSE(info->columnDefaultVisibleForRole(DFileSystemModel::FileLastModifiedRole));
    EXPECT_TRUE(info->columnDefaultVisibleForRole(Qt::UserRole));

    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 1) == MenuAction::DeletionDate);
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 2) == MenuAction::SourcePath);
    EXPECT_TRUE(info->menuActionByColumnRole(Qt::UserRole) == MenuAction::Unknow);
}

TEST_F(TestShareFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(2 == info->menuActionList(DAbstractFileInfo::SpaceArea).count());
    EXPECT_FALSE(2 == info->menuActionList(DAbstractFileInfo::SingleFile).count());
    EXPECT_TRUE(3 == info->menuActionList(DAbstractFileInfo::MultiFiles).count());
    EXPECT_TRUE(0 == info->disableMenuActionList().count());
}
