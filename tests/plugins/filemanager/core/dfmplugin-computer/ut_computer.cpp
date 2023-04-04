// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "stubmenueventinterface.h"
#include "plugins/filemanager/core/dfmplugin-computer/computer.h"
#include "plugins/filemanager/core/dfmplugin-computer/events/computereventreceiver.h"
#include "plugins/filemanager/core/dfmplugin-computer/watcher/computeritemwatcher.h"
#include "plugins/filemanager/core/dfmplugin-computer/utils/computerutils.h"

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>
#include <functional>

DPCOMPUTER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_Computer : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&ComputerItemWatcher::initAppWatcher, [] { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&ComputerItemWatcher::initConn, [] { __DBG_STUB_INVOKE__ });
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    static Computer ins;
};

Computer UT_Computer::ins;

TEST_F(UT_Computer, Initialize)
{
    stub.set_lamda(&Computer::bindEvents, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Computer::followEvents, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins.initialize());
}

TEST_F(UT_Computer, Start)
{
    shared_stubs::stubAllMenuInterfaces(&stub);

    typedef bool (dpf::EventDispatcherManager::*Subscribe)(const QString &, const QString &, ComputerEventReceiver *, decltype(&ComputerEventReceiver::handleItemEject));
    auto subscribe = static_cast<Subscribe>(&dpf::EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [] { __DBG_STUB_INVOKE__ return true; });

    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &, QString);
    auto push1 = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    typedef QVariant (dpf::EventChannelManager::*Push2)(const QString &, const QString &, QString, QString &&);
    auto push2 = static_cast<Push2>(&dpf::EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ins.start());
    EXPECT_TRUE(ins.start());
}

TEST_F(UT_Computer, Stop)
{
    EXPECT_NO_FATAL_FAILURE(ins.stop());
    EXPECT_NO_FATAL_FAILURE(ins.stop());
}

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

TEST_F(UT_Computer, OnWindowOpened)
{
    auto win = new FileManagerWindow(QUrl::fromLocalFile("/"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&] { __DBG_STUB_INVOKE__ return win; });

    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, CustomViewExtensionView, QString &&);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(111));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-111));

    stub.set_lamda(&FileManagerWindow::workSpace, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });
    stub.set_lamda(&FileManagerWindow::titleBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });
    stub.set_lamda(&ComputerItemWatcher::startQueryItems, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Computer::addComputerToSidebar, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Computer::regComputerToSearch, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(111));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-111));
}

TEST_F(UT_Computer, AddComputerToSidebar)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, int, QUrl &&, QVariantMap &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.addComputerToSidebar());
}

TEST_F(UT_Computer, RegComputerCrumbToTitleBar)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString, QVariantMap &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.regComputerCrumbToTitleBar());
}

TEST_F(UT_Computer, RegComputerToSearch)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString, QVariantMap &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.regComputerToSearch());
}

TEST_F(UT_Computer, BindEvents)
{
    typedef bool (dpf::EventChannelManager::*Connect1)(const QString &, const QString &, ComputerEventReceiver *, decltype(&ComputerEventReceiver::setContextMenuEnable));
    auto conn1 = static_cast<Connect1>(&dpf::EventChannelManager::connect);
    stub.set_lamda(conn1, [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (dpf::EventChannelManager::*Connect2)(const QString &, const QString &, ComputerItemWatcher *, decltype(&ComputerItemWatcher::addDevice));
    auto conn2 = static_cast<Connect2>(&dpf::EventChannelManager::connect);
    stub.set_lamda(conn2, [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (dpf::EventChannelManager::*Connect3)(const QString &, const QString &, ComputerItemWatcher *, decltype(&ComputerItemWatcher::removeDevice));
    auto conn3 = static_cast<Connect3>(&dpf::EventChannelManager::connect);
    stub.set_lamda(conn3, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(ins.bindEvents());
}

Q_DECLARE_METATYPE(QList<QVariantMap> *)
TEST_F(UT_Computer, FollowEvents)
{
    typedef bool (dpf::EventSequenceManager::*Follow1)(const QString &, const QString &, ComputerEventReceiver *, decltype(&ComputerEventReceiver::handleSepateTitlebarCrumb));
    auto f1 = static_cast<Follow1>(&dpf::EventSequenceManager::follow);
    stub.set_lamda(f1, [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (dpf::EventSequenceManager::*Follow2)(const QString &, const QString &, ComputerEventReceiver *, decltype(&ComputerEventReceiver::handleSortItem));
    auto f2 = static_cast<Follow2>(&dpf::EventSequenceManager::follow);
    stub.set_lamda(f2, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
}
