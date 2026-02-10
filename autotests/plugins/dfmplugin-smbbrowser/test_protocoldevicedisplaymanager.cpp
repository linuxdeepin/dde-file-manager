// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/protocoldevicedisplaymanager.h"
#include "displaycontrol/protocoldevicedisplaymanager_p.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/menu/virtualentrymenuscene.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"
#include "typedefines.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/device/deviceutils.h>

#include <dfm-framework/event/event.h>

#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QString>
#include <QList>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_ProtocolDeviceDisplayManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&ProtocolDeviceDisplayManagerPrivate::init, [] { __DBG_STUB_INVOKE__ });
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ProtocolDeviceDisplayManager, Instance)
{
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance());
    EXPECT_EQ(ProtocolDeviceDisplayManager::instance(), ProtocolDeviceDisplayManager::instance());
}

TEST_F(UT_ProtocolDeviceDisplayManager, DisplayMode)
{
    EXPECT_EQ(SmbDisplayMode::kSeperate, ProtocolDeviceDisplayManager::instance()->displayMode());
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->displayMode = SmbDisplayMode::kAggregation);
    EXPECT_EQ(SmbDisplayMode::kAggregation, ProtocolDeviceDisplayManager::instance()->displayMode());
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->displayMode = SmbDisplayMode::kSeperate);
}

TEST_F(UT_ProtocolDeviceDisplayManager, IsShowOfflineItem)
{
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->isShowOfflineItem());
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->showOffline = true);
    EXPECT_TRUE(ProtocolDeviceDisplayManager::instance()->isShowOfflineItem());
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->showOffline = false);
}

TEST_F(UT_ProtocolDeviceDisplayManager, HookItemInsert)
{
    bool support = false;

    typedef bool (ProtocolDeviceDisplayManagerPrivate::*IsSupportVEntry)(const QUrl &);
    auto func = static_cast<IsSupportVEntry>(&ProtocolDeviceDisplayManagerPrivate::isSupportVEntry);
    stub.set_lamda(func, [&] { __DBG_STUB_INVOKE__ return support; });
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->hookItemInsert(QUrl::fromLocalFile("/home")));
    support = true;
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->hookItemInsert(QUrl::fromLocalFile("/home")));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->displayMode = SmbDisplayMode::kAggregation);
    stub.set_lamda(ui_ventry_calls::addAggregatedItemForSeperatedOnlineItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(ProtocolDeviceDisplayManager::instance()->hookItemInsert(QUrl::fromLocalFile("/home")));
}

TEST_F(UT_ProtocolDeviceDisplayManager, HookItemsFilter)
{
    SmbDisplayMode mode = SmbDisplayMode::kSeperate;
    stub.set_lamda(&ProtocolDeviceDisplayManager::displayMode, [&] { __DBG_STUB_INVOKE__ return mode; });
    stub.set_lamda(&ProtocolDeviceDisplayManager::isShowOfflineItem, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(ui_ventry_calls::addSeperatedOfflineItems, [] { __DBG_STUB_INVOKE__ });
    QList<QUrl> lst;
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->hookItemsFilter(&lst));

    mode = SmbDisplayMode::kAggregation;
    stub.set_lamda(&ProtocolDeviceDisplayManagerPrivate::removeAllSmb, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ui_ventry_calls::addAggregatedItems, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(ProtocolDeviceDisplayManager::instance()->hookItemsFilter(&lst));
}

TEST_F(UT_ProtocolDeviceDisplayManager, OnDevMounted)
{
    bool isSamba = false;
    stub.set_lamda(&ProtocolUtils::isSMBFile, [&] { __DBG_STUB_INVOKE__ return isSamba; });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevMounted("1234", "1234"));

    isSamba = true;
    bool showOffline = false;
    stub.set_lamda(&ProtocolDeviceDisplayManager::isShowOfflineItem, [&] { __DBG_STUB_INVOKE__ return showOffline; });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevMounted("1234", "1234"));

    showOffline = true;
    stub.set_lamda(computer_sidebar_event_calls::callItemRemove, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryDbHandler::saveAggregatedAndSperated, [] { __DBG_STUB_INVOKE__ });
    typedef QString (*GetName)(const QUrl &);
    auto f = static_cast<GetName>(protocol_display_utilities::getDisplayNameOf);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return "Hello"; });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevMounted("1234", "1234"));
}

TEST_F(UT_ProtocolDeviceDisplayManager, OnDevUnmounted)
{
    bool isSamba = false;
    stub.set_lamda(&ProtocolUtils::isSMBFile, [&] { __DBG_STUB_INVOKE__ return isSamba; });
    bool showOffline = true;
    stub.set_lamda(&ProtocolDeviceDisplayManager::isShowOfflineItem, [&] { __DBG_STUB_INVOKE__ return showOffline; });
    SmbDisplayMode mode = SmbDisplayMode::kSeperate;
    stub.set_lamda(&ProtocolDeviceDisplayManager::displayMode, [&] { __DBG_STUB_INVOKE__ return mode; });

    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("1234"));

    isSamba = true;
    showOffline = true;
    mode = SmbDisplayMode::kSeperate;
    typedef QString (*GetStdSmbPath)(const QString &);
    auto func = static_cast<GetStdSmbPath>(protocol_display_utilities::getStandardSmbPath);
    stub.set_lamda(func, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    bool hasOffline = false;
    stub.set_lamda(&VirtualEntryDbHandler::hasOfflineEntry, [&] { __DBG_STUB_INVOKE__ return hasOffline; });
    stub.set_lamda(computer_sidebar_event_calls::callItemAdd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("1234"));
    hasOffline = true;
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("1234"));

    mode = SmbDisplayMode::kAggregation;
    showOffline = true;
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("1234"));

    showOffline = false;
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://2.3.4.5" }; });
    stub.set_lamda(protocol_display_utilities::getStandardSmbPaths, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://2.3.4.5" }; });
    stub.set_lamda(computer_sidebar_event_calls::callItemRemove, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("smb://1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDevUnmounted("smb://2.3.4.5"));
}

TEST_F(UT_ProtocolDeviceDisplayManager, OnDConfigChanged)
{
    stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&ProtocolDeviceDisplayManagerPrivate::onShowOfflineChanged, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDConfigChanged("aaa", "bbb"));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onDConfigChanged("org.deepin.dde.file-manager", "dfm.samba.permanent"));
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->d->showOffline);
}

TEST_F(UT_ProtocolDeviceDisplayManager, OnJsonConfigChanged)
{
    stub.set_lamda(&ProtocolDeviceDisplayManagerPrivate::onDisplayModeChanged, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onJsonConfigChanged("a", "b", "c"));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onJsonConfigChanged("GenericAttribute", "MergeTheEntriesOfSambaSharedFolders", "c"));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onJsonConfigChanged("GenericAttribute", "MergeTheEntriesOfSambaSharedFolders", true));
    EXPECT_EQ(SmbDisplayMode::kAggregation, ProtocolDeviceDisplayManager::instance()->d->displayMode);
}

TEST_F(UT_ProtocolDeviceDisplayManager, OnMenuSceneAdded)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString, const QString &);
    auto func = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(func, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onMenuSceneAdded("hello"));
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->onMenuSceneAdded("ComputerMenu"));
}

class UT_ProtocolDeviceDisplayManagerPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return false; });

        typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
        auto func = static_cast<Value>(&Settings::value);
        stub.set_lamda(func, [] { __DBG_STUB_INVOKE__ return true; });
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, Init)
{
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance());
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->init());
}

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, OnDisplayModeChanged)
{
    stub.set_lamda(computer_sidebar_event_calls::callComputerRefresh, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->onDisplayModeChanged());
}

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, OnShowOfflineChanged)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4", "smb://2.3.4.5" }; });
    ProtocolDeviceDisplayManager::instance()->d->showOffline = true;
    stub.set_lamda(&VirtualEntryDbHandler::saveAggregatedAndSperated, [] { __DBG_STUB_INVOKE__ });

    typedef QString (*GetName)(const QString &);
    auto f = static_cast<GetName>(protocol_display_utilities::getDisplayNameOf);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return "Hello"; });

    f = static_cast<GetName>(protocol_display_utilities::getStandardSmbPath);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->onShowOfflineChanged());
}

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, IsSupportVEntry)
{
    ProtocolDeviceDisplayManager::instance()->d->showOffline = false;
    ProtocolDeviceDisplayManager::instance()->d->displayMode = SmbDisplayMode::kSeperate;
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->d->isSupportVEntry(QUrl::fromLocalFile("/home")));

    ProtocolDeviceDisplayManager::instance()->d->showOffline = true;
    stub.set_lamda(&ProtocolUtils::isSMBFile, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->d->isSupportVEntry(QUrl::fromLocalFile("/home")));

    stub.set_lamda(&ProtocolUtils::isSMBFile, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ProtocolDeviceDisplayManager::instance()->d->isSupportVEntry(QUrl::fromLocalFile("/home")));

    EXPECT_TRUE(ProtocolDeviceDisplayManager::instance()->d->isSupportVEntry(QUrl("entry:///home.protodev")));
}

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, RemoveAllSmb)
{
    bool support = false;
    typedef bool (ProtocolDeviceDisplayManagerPrivate::*Support)(const QUrl &);
    auto func = static_cast<Support>(&ProtocolDeviceDisplayManagerPrivate::isSupportVEntry);
    stub.set_lamda(func, [&] { __DBG_STUB_INVOKE__ return support; });
    QList<QUrl> urls { QUrl::fromLocalFile("/home") };
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->removeAllSmb(&urls));
    EXPECT_TRUE(urls.count() == 1);

    support = true;
    EXPECT_NO_FATAL_FAILURE(ProtocolDeviceDisplayManager::instance()->d->removeAllSmb(&urls));
    EXPECT_TRUE(urls.count() == 0);
}
