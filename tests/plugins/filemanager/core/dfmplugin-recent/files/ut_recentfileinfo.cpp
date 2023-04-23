// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/recentfileinfo.h"
#include "utils/recentmanager.h"
#include <dfm-base/base/application/application.h>
#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

class RecentFileInfoTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        info = new RecentFileInfo(RecentHelper::rootUrl());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
        info = nullptr;
    }

private:
    stub_ext::StubExt stub;

    RecentFileInfo *info { nullptr };
};

TEST_F(RecentFileInfoTest, exists)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(RecentFileInfoTest, permissions)
{
    QFile::Permissions flag = info->permissions();
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadGroup));
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadOwner));
    EXPECT_TRUE(flag.testFlag(QFileDevice::ReadOther));
}

TEST_F(RecentFileInfoTest, nameOf)
{
    EXPECT_EQ(info->nameOf(dfmbase::NameInfoType::kFileName), "Recent");
}

TEST_F(RecentFileInfoTest, displayOf)
{
    EXPECT_TRUE(info->displayOf(dfmbase::DisPlayInfoType::kFileDisplayName) == "Recent");
}

TEST_F(RecentFileInfoTest, urlOf)
{
    EXPECT_TRUE(info->urlOf(UrlInfoType::kUrl) == RecentHelper::rootUrl());
}

TEST_F(RecentFileInfoTest, isAttributes)
{
    EXPECT_FALSE(info->isAttributes(dfmbase::OptInfoType::kIsReadable));
    EXPECT_FALSE(info->isAttributes(dfmbase::OptInfoType::kIsWritable));
}

TEST_F(RecentFileInfoTest, canAttributes)
{
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanTrash));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRename));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRedirectionFileUrl));
}
