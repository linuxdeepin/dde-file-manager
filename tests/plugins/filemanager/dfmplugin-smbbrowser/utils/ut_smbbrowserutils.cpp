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
#include "plugins/filemanager/dfmplugin-smbbrowser/utils/smbbrowserutils.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"

#include <dfm-framework/event/channel/eventchannel.h>

#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;

class UT_SmbBrowser : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    SmbBrowserUtils *ins { SmbBrowserUtils::instance() };
};

TEST_F(UT_SmbBrowser, NetworkScheme)
{
    EXPECT_TRUE(SmbBrowserUtils::instance()->networkScheme() == "network");
}

TEST_F(UT_SmbBrowser, NetNeighborRootUrl)
{
    QUrl u;
    u.setScheme("network");
    u.setPath("/");
    EXPECT_TRUE(ins->netNeighborRootUrl() == u);
    EXPECT_TRUE(ins->netNeighborRootUrl().path() == "/");
    EXPECT_TRUE(ins->netNeighborRootUrl().scheme() == "network");
}

TEST_F(UT_SmbBrowser, Icon)
{
    //    EXPECT_TRUE(ins->icon().themeName() == "network-server-symbolic");
    EXPECT_NO_FATAL_FAILURE(ins->icon().themeName());
}

TEST_F(UT_SmbBrowser, MountSmb)
{
    EXPECT_FALSE(ins->mountSmb(0, {}));
    EXPECT_FALSE(ins->mountSmb(0, { QUrl::fromLocalFile("/hello/world") }));
    EXPECT_FALSE(ins->mountSmb(0, { QUrl::fromLocalFile("/hello/world"), QUrl::fromLocalFile("/nihao") }));

    DFMBASE_USE_NAMESPACE

    QList<bool> arg1 { false, true };
    QList<dfmmount::DeviceError> arg2 { dfmmount::DeviceError::kNoError, dfmmount::DeviceError::kGIOErrorAlreadyMounted };
    QList<QString> arg3 { "/hello/world", "" };
    stub.set_lamda(&DeviceManager::mountNetworkDeviceAsync, [&](void *, const QString &, CallbackType1 cb, int) {
        __DBG_STUB_INVOKE__
        cb(arg1.first(), arg2.first(), arg3.first());
    });
    stub.set_lamda(&DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    typedef bool (dpf::EventDispatcherManager::*Publish)(int, quint64, QUrl &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_TRUE(ins->mountSmb(0, { QUrl("smb://1.2.3.4/hello/world") }));
    arg1.takeFirst();
    EXPECT_TRUE(ins->mountSmb(0, { QUrl("smb://1.2.3.4/hello/world") }));
}
