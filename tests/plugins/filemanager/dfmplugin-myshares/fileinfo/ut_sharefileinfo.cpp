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
#include "plugins/filemanager/dfmplugin-myshares/fileinfo/sharefileinfo.h"
#include "plugins/filemanager/dfmplugin-myshares/private/sharefileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/event/channel/eventchannel.h>

#include <gtest/gtest.h>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

class UT_ShareFileInfo : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override
    {
        conStub.set_lamda(&ShareFileInfo::setProxy, [] { __DBG_STUB_INVOKE__ });
        conStub.set_lamda(InfoFactory::create<AbstractFileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
        typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
        auto pushAddr = static_cast<Push>(&dpf::EventChannelManager::push);
        conStub.set_lamda(pushAddr, [] { __DBG_STUB_INVOKE__ return QVariant(); });
        info = new ShareFileInfo(QUrl("share:///test"));
        auto d = dynamic_cast<ShareFileInfoPrivate *>(info->dptr.data());
        d->info = { { "shareName", "test" },
                    { "path", "/test" },
                    { "comment", "" },
                    { "acl", "" },
                    { "guestEnable", "" },
                    { "writable", "" } };
    }

    virtual void TearDown() override
    {
        conStub.clear();
        delete info;
        info = nullptr;
    }

    stub_ext::StubExt conStub;
    ShareFileInfo *info { nullptr };
};

TEST_F(UT_ShareFileInfo, RedirectedFileUrl)
{
    EXPECT_TRUE(info->redirectedFileUrl() == QUrl::fromLocalFile("/test"));
}

TEST_F(UT_ShareFileInfo, FileDisplayName)
{
    EXPECT_TRUE(info->fileDisplayName() == "test");
}

TEST_F(UT_ShareFileInfo, FileName)
{
    EXPECT_TRUE(info->fileName() == "test");
}

TEST_F(UT_ShareFileInfo, IsDir)
{
    EXPECT_TRUE(info->isDir());
}

TEST_F(UT_ShareFileInfo, CanRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(UT_ShareFileInfo, CanDrag)
{
    EXPECT_FALSE(info->canDrag());
}

TEST_F(UT_ShareFileInfo, IsWritable)
{
    EXPECT_TRUE(info->isWritable());
}

TEST_F(UT_ShareFileInfo, CanTag)
{
    EXPECT_FALSE(info->canTag());
}

TEST_F(UT_ShareFileInfo, Refresh)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&ShareFileInfoPrivate::refresh, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(info->refresh());
}

class UT_ShareFileInfoPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        info = new ShareFileInfo(QUrl("share:///test"));
        d = dynamic_cast<ShareFileInfoPrivate *>(info->dptr.data());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
    }

private:
    stub_ext::StubExt stub;
    ShareFileInfo *info { nullptr };
    ShareFileInfoPrivate *d { nullptr };
};

TEST_F(UT_ShareFileInfoPrivate, Refresh)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(d->refresh());
}
