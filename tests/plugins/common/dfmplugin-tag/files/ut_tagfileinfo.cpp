// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/tagfileinfo.h"
#include "utils/taghelper.h"
#include "utils/tagmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagFileInfoTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        info = new TagFileInfo(QUrl("tag:///红色"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
        info = nullptr;
    }

private:
    stub_ext::StubExt stub;

    TagFileInfo *info { nullptr };
};

TEST_F(TagFileInfoTest, exists)
{
    stub.set_lamda(&TagManager::getAllTags, []() {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> map;
        map.insert("红色", QColor("red"));
        return map;
    });
    EXPECT_TRUE(info->exists());
}

TEST_F(TagFileInfoTest, permissions)
{
    QFile::Permissions flag = info->permissions();
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadGroup));
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadOwner));
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadOther));
}

TEST_F(TagFileInfoTest, nameOf)
{
    EXPECT_EQ(info->nameOf(dfmbase::NameInfoType::kFileName), "红色");
}

TEST_F(TagFileInfoTest, displayOf)
{
    EXPECT_TRUE(info->displayOf(dfmbase::DisPlayInfoType::kFileDisplayName) == "红色");
}

TEST_F(TagFileInfoTest, urlOf)
{
    EXPECT_TRUE(info->urlOf(UrlInfoType::kUrl) == QUrl("tag:///红色"));
}

TEST_F(TagFileInfoTest, isAttributes)
{
    EXPECT_TRUE(info->isAttributes(dfmbase::OptInfoType::kIsReadable));
    EXPECT_TRUE(info->isAttributes(dfmbase::OptInfoType::kIsWritable));
}

TEST_F(TagFileInfoTest, canAttributes)
{
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanTrash));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRename));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRedirectionFileUrl));
}

TEST_F(TagFileInfoTest, fileName)
{
    EXPECT_FALSE(info->fileName().isEmpty());
}

TEST_F(TagFileInfoTest, tagName)
{
    EXPECT_FALSE(info->tagName().isEmpty());
}

TEST_F(TagFileInfoTest, fileType)
{
    EXPECT_TRUE(info->fileType() == FileInfo::FileType::kDirectory);
}
