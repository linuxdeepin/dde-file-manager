// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "smbbrowser.h"
#include "utils/smbbrowserutils.h"
#include "events/traversprehandler.h"
#include "events/smbbrowsereventreceiver.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <dfm-framework/dpf.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dfm_global_defines.h>
#include <plugins/common/dfmplugin-menu/menu_eventinterface_helper.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_smbbrowser;

class UT_SmbBrowser : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    SmbBrowser ins;
};

TEST_F(UT_SmbBrowser, Initialize)
{
    stub.set_lamda(smb_browser_utils::bindSetting, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&SmbBrowser::bindWindows, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&SmbBrowser::followEvents, [] { __DBG_STUB_INVOKE__ });
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

    stub.set_lamda(ProtocolDeviceDisplayManager::instance, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(ins.start());
}

TEST_F(UT_SmbBrowser, ContextmenuHandle)
{
    auto exec_QPoint_QAction = static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec);
    QAction *act = new QAction("hello");
    stub.set_lamda(exec_QPoint_QAction, [=] { __DBG_STUB_INVOKE__ return act; });

    typedef bool (dpf::EventDispatcherManager::*Publish)(const QString &, const QString &, QString, QList<QUrl> &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowser::contextMenuHandle(0, QUrl("network:///"), QPoint()));
    EXPECT_NO_FATAL_FAILURE(SmbBrowser::contextMenuHandle(0, QUrl(), QPoint()));
    delete act;
}

TEST_F(UT_SmbBrowser, OnWindowOpened)
{
    DFMBASE_USE_NAMESPACE
    FileManagerWindow *win = new FileManagerWindow(QUrl::fromLocalFile("/hello/world"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win] { __DBG_STUB_INVOKE__ return win; });
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });
    stub.set_lamda(&SmbBrowser::updateNeighborToSidebar, [] { __DBG_STUB_INVOKE__ });

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

TEST_F(UT_SmbBrowser, UpdateNeighborToSidebar)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int, QUrl &&, QVariantMap &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.updateNeighborToSidebar());
}

TEST_F(UT_SmbBrowser, RegisterNetworkAccessPrehandler)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString, PrehandlerFunc &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_NO_FATAL_FAILURE(ins.registerNetworkAccessPrehandler());
}

TEST_F(UT_SmbBrowser, REgisterNetworkToSearch)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString, QVariantMap &&);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ins.registerNetworkToSearch());
}

TEST_F(UT_SmbBrowser, BindWindows)
{
    stub.set_lamda(&SmbBrowser::onWindowOpened, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&FileManagerWindowsManager::windowIdList, [] { __DBG_STUB_INVOKE__ return QList<quint64> { 1, 2, 3 }; });
    EXPECT_NO_FATAL_FAILURE(ins.bindWindows());
}

TEST_F(UT_SmbBrowser, FollowEvents)
{
    typedef bool (dpf::EventSequenceManager::*Follow1)(const QString &, const QString &,
                                                       SmbBrowserEventReceiver *, decltype(&SmbBrowserEventReceiver::detailViewIcon));
    auto follow1 = static_cast<Follow1>(&dpf::EventSequenceManager::follow);
    stub.set_lamda(follow1, [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (dpf::EventSequenceManager::*Follow2)(const QString &, const QString &,
                                                       SmbBrowserEventReceiver *, decltype(&SmbBrowserEventReceiver::cancelDelete));
    auto follow2 = static_cast<Follow2>(&dpf::EventSequenceManager::follow);
    stub.set_lamda(follow2, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
}

TEST_F(UT_SmbBrowser, bug_181151_registerNetworkToSearch)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString, QVariantMap &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [](EventChannelManager *&, const QString &, const QString &, QString, QVariantMap &property) {
        EXPECT_TRUE(property.contains("Property_Key_DisableSearch"));
        EXPECT_TRUE(property["Property_Key_DisableSearch"].toBool() == true);
        __DBG_STUB_INVOKE__ return QVariant();
    });

    ins.registerNetworkToSearch();
}
