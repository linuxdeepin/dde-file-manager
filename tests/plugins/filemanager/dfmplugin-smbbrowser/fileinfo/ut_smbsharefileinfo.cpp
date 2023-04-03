// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        d = info->d.data();
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
    EXPECT_NO_FATAL_FAILURE(info->nameOf(dfmbase::NameInfoType::kFileName));
    EXPECT_TRUE(info->nameOf(dfmbase::NameInfoType::kFileName) == "HelloWorld");
}

TEST_F(UT_SmbShareFileInfo, FileDisplayName)
{
    EXPECT_NO_FATAL_FAILURE(info->displayOf(dfmbase::DisPlayInfoType::kFileDisplayName));
    EXPECT_TRUE(info->displayOf(dfmbase::DisPlayInfoType::kFileDisplayName) == "HelloWorld");
}

TEST_F(UT_SmbShareFileInfo, FileIcon)
{
    EXPECT_NO_FATAL_FAILURE(info->fileIcon());
}

TEST_F(UT_SmbShareFileInfo, IsDir)
{
    EXPECT_TRUE(info->isAttributes(dfmbase::OptInfoType::kIsDir));
}

TEST_F(UT_SmbShareFileInfo, IsReadable)
{
    EXPECT_TRUE(info->isAttributes(dfmbase::OptInfoType::kIsReadable));
}

TEST_F(UT_SmbShareFileInfo, IsWritable)
{
    EXPECT_TRUE(info->isAttributes(dfmbase::OptInfoType::kIsWritable));
}

TEST_F(UT_SmbShareFileInfo, CanDrag)
{
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanDrag));
}
