// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "events/traversprehandler.h"
#include "utils/smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QString>
#include <QTimer>
#include <QDBusInterface>
#include <QNetworkInterface>
#include <functional>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_TraversPrehandler : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_TraversPrehandler, NetworkAccessPrehandler)
{
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::networkAccessPrehandler(0, QUrl::fromLocalFile("/"), nullptr));
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::networkAccessPrehandler(0, {}, nullptr));

    stub.set_lamda(travers_prehandler::doChangeCurrentUrl, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(travers_prehandler::onSmbRootMounted, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&dfmbase::DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    int type = 0;
    stub.set_lamda(&dfmbase::DeviceManager::mountNetworkDeviceAsync, [&](void *, const QString &, dfmbase::CallbackType1 cb, int) {
        if (type == 0 && cb)
            cb(false, DFMMOUNT::OperationErrorInfo(), "/");
        else if (type == 1 && cb)
            cb(true, DFMMOUNT::OperationErrorInfo(), "");
        else if (type == 2 && cb)
            cb(false, DFMMOUNT::OperationErrorInfo(), "");
    });

    EXPECT_NO_FATAL_FAILURE(travers_prehandler::networkAccessPrehandler(0, QUrl("smb://1.2.3.4/hello/world"), nullptr));
    type = 1;
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::networkAccessPrehandler(0, QUrl("smb://1.2.3.4/hello/world"), nullptr));
    type = 2;
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::networkAccessPrehandler(0, QUrl("smb://1.2.3.4/hello/world"), nullptr));
}

TEST_F(UT_TraversPrehandler, SmbAccessPrehandler)
{
    QUrl u;
    u.setScheme("smb");
    u.setHost("localhost");

    bool serviceValid = false;
    stub.set_lamda(smb_browser_utils::checkAndEnableService, [&] { __DBG_STUB_INVOKE__ return serviceValid; });
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(travers_prehandler::networkAccessPrehandler, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(travers_prehandler::smbAccessPrehandler(0, QUrl("smb://localhost/share"), nullptr));
    serviceValid = true;
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::smbAccessPrehandler(0, QUrl("smb://localhost/share"), nullptr));
}

TEST_F(UT_TraversPrehandler, DoChangeCurrentUrl)
{
    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme("file", "/");
    InfoFactory::regClass<SyncFileInfo>("file");
    DirIteratorFactory::regClass<LocalDirIterator>("file");
    WatcherFactory::regClass<LocalFileWatcher>("file");

    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType, quint64, QUrl &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, quint64, const QUrl &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(travers_prehandler::doChangeCurrentUrl(0, "", "", {}));
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::doChangeCurrentUrl(0, "/home", "", QUrl::fromLocalFile("/")));
}

TEST_F(UT_TraversPrehandler, OnSmbRootMounted)
{
    bool showOffline = false;
    int mode = 0;
    stub.set_lamda(&ProtocolDeviceDisplayManager::isShowOfflineItem, [&] { __DBG_STUB_INVOKE__ return showOffline; });
    stub.set_lamda(&ProtocolDeviceDisplayManager::displayMode, [&] { __DBG_STUB_INVOKE__ return SmbDisplayMode(mode); });
    stub.set_lamda(&VirtualEntryDbHandler::saveData, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(protocol_display_utilities::makeVEntryUrl, [] { __DBG_STUB_INVOKE__ return QUrl(); });
    stub.set_lamda(computer_sidebar_event_calls::callItemAdd, [] { __DBG_STUB_INVOKE__ });

    auto after = [] {};
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::onSmbRootMounted("", after));
    showOffline = true;
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::onSmbRootMounted("", after));
    mode = 1;
    EXPECT_NO_FATAL_FAILURE(travers_prehandler::onSmbRootMounted("", after));
}

class UT_PrehandlerUtils : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_PrehandlerUtils, SplitMountSource)
{
    EXPECT_EQ("hello", prehandler_utils::splitMountSource("hello"));
    QString sub;
    EXPECT_EQ("smb://1.2.3.4/hello", prehandler_utils::splitMountSource("smb://1.2.3.4/hello/", &sub));
    EXPECT_TRUE(sub.isEmpty());

    sub.clear();
    EXPECT_EQ("smb://1.2.3.4/hello", prehandler_utils::splitMountSource("smb://1.2.3.4/hello/world", &sub));
    EXPECT_EQ("world", sub);

    sub.clear();
    EXPECT_NE("smb://1.2.3.4/helloworld", prehandler_utils::splitMountSource("smb://1.2.3.4/hello/", &sub));
    EXPECT_NE("smb://1.2.3.4worldhello", prehandler_utils::splitMountSource("smb://1.2.3.4/hello/world", &sub));

    sub.clear();
    EXPECT_TRUE(prehandler_utils::splitMountSource("").isEmpty());
    EXPECT_NO_FATAL_FAILURE(prehandler_utils::splitMountSource("", nullptr));
}
