// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/utilities/protocoldisplayutilities.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-framework/event/event.h>
#include <dfm-base/base/schemefactory.h>

#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QList>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_ProtocolDisplayUtilities : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ProtocolDeviceDisplayManager::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ProtocolDisplayUtilities, PDU_MakeVEntry)
{
    QString uri = "smb://1.2.3.4/hello/world";
    QUrl url;
    url.setScheme("entry");
    url.setPath(uri + ".ventry");

    EXPECT_EQ(url, protocol_display_utilities::makeVEntryUrl(uri));
    url.setScheme("file");
    EXPECT_NE(url, protocol_display_utilities::makeVEntryUrl(uri));
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_GetMountedSmb)
{
    stub.set_lamda(&DeviceProxyManager::getAllProtocolIds, [] {
        __DBG_STUB_INVOKE__
        return QStringList { "smb://1.2.3.4/hello", "ftp://1.2.3.4" };
    });

    QStringList ret = protocol_display_utilities::getMountedSmb();
    EXPECT_TRUE(ret.count() == 1);
    EXPECT_TRUE(ret.first() == "smb://1.2.3.4/hello");
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_GetStandardSmbPaths)
{
    QString (*getStdSmbPath_QString)(const QString &) = protocol_display_utilities::getStandardSmbPath;
    stub.set_lamda(getStdSmbPath_QString, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });

    QStringList cases { "1234", "12344", "smb" };
    EXPECT_EQ(cases.count(), protocol_display_utilities::getStandardSmbPaths(cases).count());
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_GetSmbHostPath)
{
    QString (*getStdSmbPath_QString)(const QString &) = protocol_display_utilities::getStandardSmbPath;
    stub.set_lamda(getStdSmbPath_QString, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });

    EXPECT_EQ("smb://1.2.3.4", protocol_display_utilities::getSmbHostPath("xxxx"));
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_GetStandardSmbPath)
{
    QUrl u("entry://smb://1.2.3.4/hello.ventry");
    EXPECT_EQ("", protocol_display_utilities::getStandardSmbPath(u));

    u.setPath("smb://1.2.3.4/hello.protodev");
    QString (*getStdSmbPath_QString)(const QString &) = protocol_display_utilities::getStandardSmbPath;
    stub.set_lamda(getStdSmbPath_QString, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });
    EXPECT_EQ("smb://1.2.3.4/hello", protocol_display_utilities::getStandardSmbPath(u));
    stub.clear();

    EXPECT_EQ("1234", protocol_display_utilities::getStandardSmbPath("1234"));

    stub.set_lamda(DeviceUtils::parseSmbInfo, [] { __DBG_STUB_INVOKE__ return false; });
    QString input("file:///media/user/smbmounts/hello world");
    EXPECT_EQ(input, protocol_display_utilities::getStandardSmbPath(input));

    QString smbPort = "";
    stub.set_lamda(DeviceUtils::parseSmbInfo, [&](const QString &, QString &host, QString &share, QString *port) {
        __DBG_STUB_INVOKE__
        host = "1.2.3.4";
        share = "smb";
        if (port) *port = smbPort;
        return true;
    });
    EXPECT_EQ("smb://1.2.3.4/smb/", protocol_display_utilities::getStandardSmbPath(input));
    smbPort = "448";
    EXPECT_EQ("smb://1.2.3.4:448/smb/", protocol_display_utilities::getStandardSmbPath(input));
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_GetDisplayNameOf)
{
    EntryEntityFactor::registCreator<ProtocolVirtualEntryEntity>(kComputerProtocolSuffix);
    QString name("share on 1.2.3.4");
    stub.set_lamda(&EntryFileInfo::displayName, [&] { __DBG_STUB_INVOKE__ return name; });
    EXPECT_EQ(name, protocol_display_utilities::getDisplayNameOf("smb://1.2.3.4/share"));
    EXPECT_EQ(name, protocol_display_utilities::getDisplayNameOf(QUrl("entry://smb://1.2.3.4/share.ventry")));
}

TEST_F(UT_ProtocolDisplayUtilities, PDU_HasMountedShareOf)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] {
        __DBG_STUB_INVOKE__
        return QStringList { "smb://1.2.3.4/share" };
    });
    EXPECT_TRUE(protocol_display_utilities::hasMountedShareOf("smb://1.2.3.4"));
    EXPECT_FALSE(protocol_display_utilities::hasMountedShareOf("smb://2.3.4.5"));
}

TEST_F(UT_ProtocolDisplayUtilities, CSEC_CallItemAdd)
{
    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &,
                                                        QString, const QUrl &, int &&, bool &&);
    auto push1 = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    typedef QVariant (dpf::EventChannelManager::*Push2)(const QString &, const QString &,
                                                        QUrl, QVariantMap &);
    auto push2 = static_cast<Push2>(&dpf::EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(computer_sidebar_event_calls::callItemAdd(QUrl("entry://smb://1.2.3.4/smb.ventry")));
}

TEST_F(UT_ProtocolDisplayUtilities, CSEC_CallItemRemove)
{
    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &,
                                                        QUrl);
    auto push1 = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(computer_sidebar_event_calls::callItemRemove(QUrl("entry://smb://1.2.3.4/smb.ventry")));
}

TEST_F(UT_ProtocolDisplayUtilities, CSEC_CallComputerRefresh)
{
    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &,
                                                        QUrl);
    auto push1 = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    typedef QVariant (dpf::EventChannelManager::*Push2)(const QString &, const QString &);
    auto push2 = static_cast<Push2>(&dpf::EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    stub.set_lamda(&VirtualEntryDbHandler::allSmbIDs, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4/hello" }; });
    EXPECT_NO_FATAL_FAILURE(computer_sidebar_event_calls::callComputerRefresh());
}

TEST_F(UT_ProtocolDisplayUtilities, CSEC_CallForgetPasswd)
{
    typedef QVariant (dpf::EventChannelManager::*Push1)(const QString &, const QString &,
                                                        QString);
    auto push1 = static_cast<Push1>(&dpf::EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(computer_sidebar_event_calls::callForgetPasswd("smb://1.2.3.4/hello"));
}

TEST_F(UT_ProtocolDisplayUtilities, CSEC_SidebarMenuCall) { }

TEST_F(UT_ProtocolDisplayUtilities, UVC_AddAggregatedItemForSeperatedOnlineItem)
{
    QString (*getStdSmbPath_QUrl)(const QUrl &) = protocol_display_utilities::getStandardSmbPath;
    stub.set_lamda(getStdSmbPath_QUrl, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });
    stub.set_lamda(protocol_display_utilities::getSmbHostPath, [] { __DBG_STUB_INVOKE__ return ""; });
    EXPECT_NO_FATAL_FAILURE(ui_ventry_calls::addAggregatedItemForSeperatedOnlineItem(QUrl("entry://smb://1.2.3.4/share.ventry")));

    stub.set_lamda(protocol_display_utilities::getSmbHostPath, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    stub.set_lamda(computer_sidebar_event_calls::callItemAdd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ui_ventry_calls::addAggregatedItemForSeperatedOnlineItem(QUrl("entry://smb://1.2.3.4/share.ventry")));
}

TEST_F(UT_ProtocolDisplayUtilities, UVC_AddAggregatedItems)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4/hello" }; });
    stub.set_lamda(protocol_display_utilities::getStandardSmbPaths, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4/hello" }; });
    stub.set_lamda(&VirtualEntryDbHandler::allSmbIDs, [] { __DBG_STUB_INVOKE__ return QStringList { "" }; });
    stub.set_lamda(computer_sidebar_event_calls::callItemAdd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ui_ventry_calls::addAggregatedItems());
}

TEST_F(UT_ProtocolDisplayUtilities, UVC_AddSeperatedOfflineItems)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4/hello" }; });
    stub.set_lamda(protocol_display_utilities::getStandardSmbPaths, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4/hello" }; });
    stub.set_lamda(&VirtualEntryDbHandler::allSmbIDs, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://2.3.4.5/share" }; });
    stub.set_lamda(computer_sidebar_event_calls::callItemAdd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ui_ventry_calls::addSeperatedOfflineItems());
}
