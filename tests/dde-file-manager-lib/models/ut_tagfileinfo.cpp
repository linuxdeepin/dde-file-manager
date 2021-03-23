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

#include <gtest/gtest.h>
#include "models/tagfileinfo.h"
#include "interfaces/dfmstandardpaths.h"

namespace {
class TestTagFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTagFile";
        info = new TagFileInfo(DUrl("tag:///"));
        info2 = new TagFileInfo(DUrl("tag:///test"));
    }

    void TearDown() override
    {
        std::cout << "end TestTagFile";
        delete info;
        info = nullptr;
        delete info2;
        info2 = nullptr;
    }

public:
    TagFileInfo *info, *info2;
};
} // namespace

TEST_F(TestTagFileInfo, PropertyTest)
{
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->makeAbsolute());
    EXPECT_TRUE(info->exists());
    EXPECT_FALSE(info2->exists());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info2->canRename());
    EXPECT_TRUE(info->isTaged());
    EXPECT_TRUE(info->isWritable());
    EXPECT_TRUE(info2->isWritable());
    EXPECT_FALSE(info->canRedirectionFileUrl());
    EXPECT_TRUE(info->canIteratorDir());
    EXPECT_TRUE(info->canDrop());
    EXPECT_TRUE(info->isVirtualEntry());
    EXPECT_TRUE(0 < info->permissions());
    EXPECT_TRUE((Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled) == info->fileItemDisableFlags());
    EXPECT_TRUE(0 == info->extraProperties().count());
    EXPECT_TRUE(0 < info2->extraProperties().count());
    EXPECT_TRUE(0 < info->userColumnRoles().count());
    EXPECT_TRUE(0 < info2->userColumnRoles().count());
    EXPECT_TRUE(info->supportedDragActions() & Qt::CopyAction);
    EXPECT_FALSE(info->supportedDropActions() & Qt::CopyAction);
}

TEST_F(TestTagFileInfo, UrlTest)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
    EXPECT_STREQ(DFMStandardPaths::location(DFMStandardPaths::HomePath).toStdString().c_str(),
                 info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_STREQ("/", info->redirectedFileUrl().path().toStdString().c_str());
    EXPECT_STREQ("", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestTagFileInfo, StringPropertyTest)
{
    EXPECT_STREQ("folder", info->iconName().toStdString().c_str());
    EXPECT_STREQ("-", info->sizeDisplayName().toStdString().c_str());
}

TEST_F(TestTagFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(2 == info->menuActionList(DAbstractFileInfo::SpaceArea).count());
    EXPECT_TRUE(5 == info->menuActionList(DAbstractFileInfo::SingleFile).count());
}

TEST_F(TestTagFileInfo, tstGetUrlByFileName)
{
    EXPECT_TRUE(info->getUrlByNewFileName("test").isValid());
}
