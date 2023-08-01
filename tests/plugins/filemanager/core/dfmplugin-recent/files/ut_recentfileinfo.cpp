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
        info2 = new RecentFileInfo(QUrl("test"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
        info = nullptr;
        delete info2;
        info2 = nullptr;
    }

protected:
    stub_ext::StubExt stub;

    RecentFileInfo *info { nullptr };
    RecentFileInfo *info2 { nullptr };
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
    EXPECT_EQ(info2->nameOf(dfmbase::NameInfoType::kFileName), "Recent");
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
    EXPECT_FALSE(info->isAttributes(dfmbase::OptInfoType::kIsHidden));
}

TEST_F(RecentFileInfoTest, canAttributes)
{
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanTrash));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRename));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanRedirectionFileUrl));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kCanTrash));
    EXPECT_FALSE(info->canAttributes(dfmbase::CanableInfoType::kUnknowFileCanInfo));
}

TEST_F(RecentFileInfoTest, customData)
{
    using namespace dfmbase::Global;
    stub.set_lamda(VADDR(RecentFileInfo, urlOf), [] {
        return QUrl("test");
    });
    stub.set_lamda(VADDR(RecentFileInfo, timeOf), [] {
        return QVariant(QDateTime::currentDateTime());
    });
    EXPECT_FALSE(info->customData(kItemFilePathRole).toString().isEmpty());
    EXPECT_FALSE(info->customData(kItemFileLastReadRole).toString().isEmpty());
    EXPECT_FALSE(!info->customData(1).toString().isEmpty());
}
