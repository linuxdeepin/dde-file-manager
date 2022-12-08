/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/fileinfo/smbsharefileinfo.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/private/smbsharefileinfo_p.h"

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;

class UT_SmbShareFileInfo : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        info = new SmbShareFileInfo(QUrl::fromLocalFile("/hello/world"));
        d = dynamic_cast<SmbShareFileInfoPrivate *>(info->dptr.data());

        d->node = { "/hello/world", "HelloWorld", "folder-remote" };
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
        info = nullptr;
    }

private:
    stub_ext::StubExt stub;

    SmbShareFileInfo *info { nullptr };
    SmbShareFileInfoPrivate *d { nullptr };
};

TEST_F(UT_SmbShareFileInfo, FileName)
{
    EXPECT_NO_FATAL_FAILURE(info->nameInfo(NameInfo::kFileName));
    EXPECT_TRUE(info->nameInfo(NameInfo::kFileName) == "HelloWorld");
}

TEST_F(UT_SmbShareFileInfo, FileDisplayName)
{
    EXPECT_NO_FATAL_FAILURE(info->displayInfo(DisPlay::kFileDisplayName));
    EXPECT_TRUE(info->displayInfo(DisPlay::kFileDisplayName) == "HelloWorld");
}

TEST_F(UT_SmbShareFileInfo, FileIcon)
{
    EXPECT_NO_FATAL_FAILURE(info->fileIcon());
}

TEST_F(UT_SmbShareFileInfo, IsDir)
{
    EXPECT_TRUE(info->isAttributes(IsInfo::kIsDir));
}

TEST_F(UT_SmbShareFileInfo, IsReadable)
{
    EXPECT_TRUE(info->isAttributes(IsInfo::kIsReadable));
}

TEST_F(UT_SmbShareFileInfo, IsWritable)
{
    EXPECT_TRUE(info->isAttributes(IsInfo::kIsWritable));
}

TEST_F(UT_SmbShareFileInfo, CanDrag)
{
    EXPECT_FALSE(info->canAttributes(CanInfo::kCanDrag));
}
