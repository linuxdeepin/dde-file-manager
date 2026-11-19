// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/utils/networkutils.h>
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
    stub.set_lamda(static_cast<bool (NetworkUtils::*)(const QUrl &, int)>(&NetworkUtils::checkFtpOrSmbBusy),
                   [](NetworkUtils *, const QUrl &, int) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

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

    stub.set_lamda(&DialogManager::showUnableToVistDir, [](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return 0;
    });

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

TEST_F(UT_TraversPrehandler, NetworkAccessPrehandler_UnreachableHost_AbortsMount)
{
    NetworkUtils::instance()->clearCache();

    QString probedHost;
    QStringList probedPorts;
    int probedMsecs { 0 };
    using CheckNetFunc = bool (NetworkUtils::*)(const QString &, QStringList, int);
    stub.set_lamda(static_cast<CheckNetFunc>(&NetworkUtils::checkNetConnection),
                   [&](NetworkUtils *, const QString &host, QStringList ports, int msecs) {
                       __DBG_STUB_INVOKE__
                       probedHost = host;
                       probedPorts = ports;
                       probedMsecs = msecs;
                       return false;
                   });

    bool errorShown = false;
    stub.set_lamda(&DialogManager::showUnableToVistDir, [&errorShown](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
        errorShown = true;
        return 0;
    });

    bool mountCalled = false;
    stub.set_lamda(&DeviceManager::mountNetworkDeviceAsync, [&mountCalled](void *, const QString &, dfmbase::CallbackType1, int) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    travers_prehandler::networkAccessPrehandler(0, QUrl("ftp://1.2.3.4/wenjian/jj"), nullptr);

    EXPECT_FALSE(mountCalled);
    EXPECT_TRUE(errorShown);
    EXPECT_EQ(probedHost, QString("1.2.3.4"));
    EXPECT_EQ(probedPorts, QStringList { "21" });
    EXPECT_EQ(probedMsecs, 1000);
}

TEST_F(UT_TraversPrehandler, DoChangeCurrentUrl_MissingSubDirectory_ShowsErrorAndDoesNotNavigate)
{
    stub.set_lamda(static_cast<QSharedPointer<FileInfo> (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new SyncFileInfo(url));
                   });

    bool errorShown = false;
    stub.set_lamda(&DialogManager::showUnableToVistDir, [&errorShown](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
        errorShown = true;
        return 0;
    });

    bool navigationPublished = false;
    typedef bool (dpf::EventDispatcherManager::*PublishFunc)(dpf::EventType, quint64, QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&dpf::EventDispatcherManager::publish),
                   [&navigationPublished](dpf::EventDispatcherManager *, dpf::EventType, quint64, QUrl &) {
                       __DBG_STUB_INVOKE__
                       navigationPublished = true;
                       return true;
                   });

    travers_prehandler::doChangeCurrentUrl(0, "/tmp", "dfm-ut-definitely-missing-dir-4755", QUrl("ftp://1.2.3.4/wenjian/jj"));

    EXPECT_TRUE(errorShown);
    EXPECT_FALSE(navigationPublished);
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

    // ftp/sftp: mount source is the host only, subPath is the rest
    sub.clear();
    EXPECT_EQ("ftp://1.2.3.4", prehandler_utils::splitMountSource("ftp://1.2.3.4/path", &sub));
    EXPECT_EQ("path", sub);

    sub.clear();
    EXPECT_EQ("sftp://1.2.3.4", prehandler_utils::splitMountSource("sftp://1.2.3.4/path/to/file", &sub));
    EXPECT_EQ("path/to/file", sub);

    // ftp/sftp with no path: mount source is host, subPath is empty
    sub.clear();
    EXPECT_EQ("sftp://1.2.3.4", prehandler_utils::splitMountSource("sftp://1.2.3.4", &sub));
    EXPECT_TRUE(sub.isEmpty());

    // ftp/sftp with trailing slash only
    sub.clear();
    EXPECT_EQ("ftp://1.2.3.4", prehandler_utils::splitMountSource("ftp://1.2.3.4/", &sub));
    EXPECT_TRUE(sub.isEmpty());

    // ftp/sftp with ?query (e.g. charset=gbk): query must be preserved on mount source
    sub.clear();
    EXPECT_EQ("sftp://1.2.3.4?charset=gbk", prehandler_utils::splitMountSource("sftp://1.2.3.4/path?charset=gbk", &sub));
    EXPECT_EQ("path", sub);

    sub.clear();
    EXPECT_EQ("ftp://1.2.3.4?charset=gbk", prehandler_utils::splitMountSource("ftp://1.2.3.4/dir/file?charset=gbk", &sub));
    EXPECT_EQ("dir/file", sub);

    sub.clear();
    EXPECT_EQ("smb://1.2.3.4/share?charset=gbk", prehandler_utils::splitMountSource("smb://1.2.3.4/share?charset=gbk", &sub));
    EXPECT_TRUE(sub.isEmpty());

    // uppercase scheme: regex is case-insensitive
    sub.clear();
    EXPECT_EQ("SFTP://1.2.3.4", prehandler_utils::splitMountSource("SFTP://1.2.3.4/path", &sub));
    EXPECT_EQ("path", sub);

    sub.clear();
    EXPECT_EQ("FTP://1.2.3.4", prehandler_utils::splitMountSource("FTP://1.2.3.4/dir", &sub));
    EXPECT_EQ("dir", sub);

    // uppercase scheme with query
    sub.clear();
    EXPECT_EQ("SFTP://1.2.3.4?charset=utf-8", prehandler_utils::splitMountSource("SFTP://1.2.3.4/sub?charset=utf-8", &sub));
    EXPECT_EQ("sub", sub);

    // non-matching scheme returns source unchanged
    sub.clear();
    EXPECT_EQ("nfs://1.2.3.4/path", prehandler_utils::splitMountSource("nfs://1.2.3.4/path", &sub));
    EXPECT_TRUE(sub.isEmpty());
}
