// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"

#include "stubmenueventinterface.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/smbbrowser.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"

#include <dfm-framework/event/eventchannel.h>

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;
using namespace dpf;

class UT_SmbBrowser : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    SmbBrowser ins;
};

TEST_F(UT_SmbBrowser, Initialize)
{
    EXPECT_NO_FATAL_FAILURE(ins.initialize());
}

TEST_F(UT_SmbBrowser, Start)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, QString);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, QString, QString &&);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    shared_stubs::stubAllMenuInterfaces(&stub);

    EXPECT_NO_FATAL_FAILURE(ins.start());
}

TEST_F(UT_SmbBrowser, Stop)
{
    EXPECT_NO_FATAL_FAILURE(ins.stop());
    EXPECT_NO_FATAL_FAILURE(ins.stop());
}

TEST_F(UT_SmbBrowser, OnWindowOpened)
{
    DFMBASE_USE_NAMESPACE
    FileManagerWindow *win = new FileManagerWindow(QUrl::fromLocalFile("/hello/world"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win] { __DBG_STUB_INVOKE__ return win; });
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });
    stub.set_lamda(&SmbBrowser::addNeighborToSidebar, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(1));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(2));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(3));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-10086));

    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(1));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(2));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(3));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-10086));
    delete win;
}

TEST_F(UT_SmbBrowser, AddNeighborToSidebar)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, QUrl &&, QVariantMap &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.addNeighborToSidebar());
}

TEST_F(UT_SmbBrowser, RegisterNetworkAccessPrehandler)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString, Prehandler &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_NO_FATAL_FAILURE(ins.registerNetworkAccessPrehandler());
}

TEST_F(UT_SmbBrowser, NetworkAccessPrehandler)
{
    typedef bool (EventDispatcherManager::*Publish)(int, quint64, QUrl &&);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    DFMBASE_USE_NAMESPACE
    stub.set_lamda(&DialogManager::showErrorDialog, [] { __DBG_STUB_INVOKE__ });

    QList<bool> arg1 { true, false };
    QList<dfmmount::DeviceError> arg2 { dfmmount::DeviceError::kNoError, dfmmount::DeviceError::kGIOErrorAlreadyMounted };
    QList<QString> arg3 { "/hello/world", "" };
    stub.set_lamda(&DeviceManager::mountNetworkDeviceAsync, [&](void *, const QString &, CallbackType1 cb, int) {
        __DBG_STUB_INVOKE__
        cb(arg1.first(), arg2.first(), arg3.first());
    });

    EXPECT_NO_FATAL_FAILURE(ins.networkAccessPrehandler(0, QUrl::fromLocalFile("/hello/world"), nullptr));
    EXPECT_NO_FATAL_FAILURE(ins.networkAccessPrehandler(0, QUrl("smb://1.23.4.5/hello/world"), nullptr));

    arg3.takeFirst();
    EXPECT_NO_FATAL_FAILURE(ins.networkAccessPrehandler(0, QUrl("smb://1.23.4.5/hello/world"), [] { __DBG_STUB_INVOKE__ }));
    EXPECT_NO_FATAL_FAILURE(ins.networkAccessPrehandler(0, QUrl("smb://1.23.4.5/hello/world"), nullptr));
}
