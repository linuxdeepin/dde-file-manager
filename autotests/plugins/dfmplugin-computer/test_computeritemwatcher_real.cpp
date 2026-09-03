// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "watcher/computeritemwatcher.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"
#include "fileentity/commonentryfileentity.h"
#include "fileentity/userentryfileentity.h"
#include "fileentity/blockentryfileentity.h"
#include "fileentity/protocolentryfileentity.h"
#include "fileentity/appentryfileentity.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QSignalSpy>
#include <QUrl>
#include <QTimer>
#include <QTest>
#include <QtDBus/QDBusVariant>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;
using namespace GlobalServerDefines;
using namespace GlobalDConfDefines::ConfigPath;

class UT_ComputerItemWatcherReal : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        watcher = ComputerItemWatcher::instance();
        ASSERT_NE(watcher, nullptr);

        // Idempotent entity/scheme registrations (same set as Computer::initialize)
        EntryEntityFactor::registCreator<CommonEntryFileEntity>(SuffixInfo::kCommon);
        EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
        EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
        EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
        EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);
        UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
        InfoFactory::regClass<EntryFileInfo>(Global::Scheme::kEntry);

        // EntryFileInfo objects are really constructed (entry:// scheme),
        // only their backend behaviour is stubbed out.
        stub.set_lamda(VADDR(EntryFileInfo, exists), [](EntryFileInfo *) {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&EntryFileInfo::displayName, [](EntryFileInfo *) {
            __DBG_STUB_INVOKE__
            return QString("Real Test Device");
        });
        stub.set_lamda(&EntryFileInfo::renamable, [](EntryFileInfo *) {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Device enumeration: one block device, one protocol device
        stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [](DeviceProxyManager *, GlobalServerDefines::DeviceQueryOptions) {
            __DBG_STUB_INVOKE__
            return QStringList { "/org/freedesktop/UDisks2/block_devices/sda1" };
        });
        stub.set_lamda(&DeviceProxyManager::getAllProtocolIds, [](DeviceProxyManager *) {
            __DBG_STUB_INVOKE__
            return QStringList { "smb://192.168.1.10/share" };
        });

        // DConfig-backed UUID lookups go through dfm-mount; hand out empty sets
        stub.set_lamda(&ComputerUtils::allValidBlockUUIDs, []() {
            __DBG_STUB_INVOKE__
            return QStringList {};
        });
        stub.set_lamda(&ComputerUtils::blkDevUrlByUUIDs, [](const QList<QString> &) {
            __DBG_STUB_INVOKE__
            return QList<QUrl> {};
        });
        stub.set_lamda(&ComputerUtils::shouldSystemPartitionHide, []() {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ComputerUtils::shouldLoopPartitionsHide, []() {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stable ordering for the disk area sort
        stub.set_lamda(static_cast<bool (*)(DFMEntryFileInfoPointer, DFMEntryFileInfoPointer)>(
                               &ComputerUtils::sortItem),
                       [](const DFMEntryFileInfoPointer &, const DFMEntryFileInfoPointer &) {
                           __DBG_STUB_INVOKE__
                           return false;
                       });

        // Mimic the finished query so addDevice() runs its body directly
        watcher->isItemQueryFinished = true;
    }

    virtual void TearDown() override
    {
        // Do not leak state into other suites through the singleton
        watcher->isItemQueryFinished = false;
        watcher->initedDatas.clear();
        watcher->thirdItemList.clear();
        watcher->sidebarInfos.clear();
        watcher->computerInfos.clear();
        watcher->routeMapper.clear();
        watcher->groupIds.clear();
        watcher->pendingSidebarDevUrls.clear();
        // And do not leak the emissions themselves: e.g. the shared static
        // ComputerModel (kCptModelIns) stays connected to this singleton and
        // would accumulate our items across suites.
        watcher->disconnect();
        stub.clear();
    }

    static QUrl blockUrl(const QString &id)
    {
        return ComputerUtils::makeBlockDevUrl(id);
    }

    stub_ext::StubExt stub;
    ComputerItemWatcher *watcher = nullptr;
};

TEST_F(UT_ComputerItemWatcherReal, Items_FullPipeline_BuildsAllGroups)
{
    ComputerDataList data = watcher->items();

    EXPECT_FALSE(data.isEmpty());

    bool hasDirsGroup = false;
    bool hasDisksGroup = false;
    bool hasBlockDev = false;
    bool hasProtocolDev = false;
    for (const auto &item : data) {
        if (item.shape == ComputerItemData::kSplitterItem && item.itemName == watcher->diskGroup())
            hasDisksGroup = true;
        if (item.shape == ComputerItemData::kSplitterItem && item.itemName == watcher->userDirGroup())
            hasDirsGroup = true;
        if (item.url.isValid() && item.url.path().endsWith(SuffixInfo::kBlock))
            hasBlockDev = true;
        if (item.url.isValid() && item.url.path().endsWith(SuffixInfo::kProtocol))
            hasProtocolDev = true;
    }

    EXPECT_TRUE(hasDirsGroup);
    EXPECT_TRUE(hasDisksGroup);
    EXPECT_TRUE(hasBlockDev);
    EXPECT_TRUE(hasProtocolDev);
}

TEST_F(UT_ComputerItemWatcherReal, StartQueryItems_Sync_EmitsFinishedAndFillsCache)
{
    QSignalSpy spy(watcher, &ComputerItemWatcher::itemQueryFinished);

    watcher->startQueryItems(false);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(watcher->getInitedItems().isEmpty());
    // the block device is queued as pending sidebar entry and its info is built
    const QUrl sda = blockUrl("/org/freedesktop/UDisks2/block_devices/sda1");
    EXPECT_TRUE(watcher->sidebarInfos.contains(sda));
    EXPECT_FALSE(watcher->sidebarInfos.value(sda).isEmpty());
}

TEST_F(UT_ComputerItemWatcherReal, AddAndRemoveDevice_RealFlow_EmitsSignals)
{
    QUrl devUrl = blockUrl("/org/freedesktop/UDisks2/block_devices/sdb1");

    QSignalSpy addSpy(watcher, &ComputerItemWatcher::itemAdded);
    watcher->addDevice(watcher->diskGroup(), devUrl, ComputerItemData::kLargeItem, false);
    // one emission from addGroup (group splitter) + one from onDeviceAdded
    EXPECT_EQ(addSpy.count(), 2);

    bool found = false;
    for (const auto &item : watcher->getInitedItems())
        if (item.url == devUrl)
            found = true;
    EXPECT_TRUE(found);

    QSignalSpy removeSpy(watcher, &ComputerItemWatcher::itemRemoved);
    watcher->removeDevice(devUrl);
    EXPECT_EQ(removeSpy.count(), 1);
}

TEST_F(UT_ComputerItemWatcherReal, AddDevice_WhileQueryPending_DeferredUntilFinished)
{
    watcher->isItemQueryFinished = false;

    QUrl devUrl = blockUrl("/org/freedesktop/UDisks2/block_devices/sdc9");
    QSignalSpy addSpy(watcher, &ComputerItemWatcher::itemAdded);
    watcher->addDevice(watcher->diskGroup(), devUrl, ComputerItemData::kLargeItem, false);
    // pending connection: nothing emitted yet
    EXPECT_EQ(addSpy.count(), 0);

    watcher->startQueryItems(false);   // emits itemQueryFinished, deferred add runs
    EXPECT_GE(addSpy.count(), 1);
}

TEST_F(UT_ComputerItemWatcherReal, SidebarItemManagement_PushesWithoutSidebar_NoCrash)
{
    QUrl url = blockUrl("/org/freedesktop/UDisks2/block_devices/sdc1");

    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    EXPECT_NO_THROW(watcher->addSidebarItem(info));
    EXPECT_NO_THROW(watcher->addSidebarItem(url, QVariantMap { { "k", "v" } }));
    EXPECT_NO_THROW(watcher->removeSidebarItem(url));
    EXPECT_NO_THROW(watcher->updateSidebarItem(url, "New Name", true));

    // flush queued updateSidebarItem from onUpdateBlockItem paths
    QTest::qWait(20);
}

TEST_F(UT_ComputerItemWatcherReal, GroupLifecycle_AddAndRemove_ReturnsConsistentIds)
{
    int gid = watcher->addGroup("Real Group");
    int gid2 = watcher->addGroup("Real Group");
    EXPECT_EQ(gid, gid2);

    EXPECT_TRUE(watcher->removeGroup("Real Group"));
    EXPECT_FALSE(watcher->removeGroup("Real Group"));
}

TEST_F(UT_ComputerItemWatcherReal, HiddenHelpers_EmptyDConfigAndNoMount_ReturnsEmpty)
{
    EXPECT_TRUE(watcher->disksHiddenByDConf().isEmpty());
    EXPECT_TRUE(watcher->disksHiddenBySettingPanel().isEmpty());
    EXPECT_TRUE(watcher->hiddenPartitions().isEmpty());

    // DConfig defaults for visibility switches
    EXPECT_FALSE(watcher->hideUserDir());
    EXPECT_FALSE(watcher->hide3rdEntries());
}

TEST_F(UT_ComputerItemWatcherReal, UrlMapper_BurnDevice_MapsToBurnUrl)
{
    const QString opticalId = "/org/freedesktop/UDisks2/block_devices/sr0";
    QUrl mnt("file:///media/cdrom");

    watcher->insertUrlMapper(opticalId, mnt);

    QUrl opticalUrl = blockUrl(opticalId);
    EXPECT_TRUE(watcher->routeMapper.contains(opticalUrl));
    EXPECT_TRUE(watcher->routeMapper.values(opticalUrl).contains(ComputerUtils::makeBurnUrl(opticalId)));
    EXPECT_TRUE(watcher->routeMapper.values(opticalUrl).contains(mnt));

    DFMEntryFileInfoPointer info(new EntryFileInfo(opticalUrl));
    EXPECT_EQ(watcher->findFinalUrl(info), ComputerUtils::makeBurnUrl(opticalId));

    // unmounted: route cleaned up
    watcher->onBlockDeviceUnmounted(opticalId);
    EXPECT_FALSE(watcher->routeMapper.contains(opticalUrl));
}

TEST_F(UT_ComputerItemWatcherReal, ReportName_AndComputerInfos_ReturnDefaults)
{
    EXPECT_EQ(watcher->reportName(QUrl("entry://x.blockdev")), QString("unknow disk"));
    EXPECT_TRUE(watcher->getComputerInfos().isEmpty());
    EXPECT_FALSE(watcher->findFinalUrl(DFMEntryFileInfoPointer()).isValid());
}

TEST_F(UT_ComputerItemWatcherReal, DeviceLifecycleSlots_NoQtService_NoCrash)
{
    // These slots are connected to DevProxyManager signals in production;
    // with stubbed enumeration they run through their full logic.
    EXPECT_NO_THROW(watcher->onBlockDeviceAdded("/org/freedesktop/UDisks2/block_devices/sdd1"));
    EXPECT_NO_THROW(watcher->onBlockDeviceRemoved("/org/freedesktop/UDisks2/block_devices/sdd1"));
    EXPECT_NO_THROW(watcher->onProtocolDeviceRemoved("smb://host/share"));
    EXPECT_NO_THROW(watcher->onProtocolDeviceUnmounted("smb://host/share"));
    EXPECT_NO_THROW(watcher->onDeviceSizeChanged("/org/freedesktop/UDisks2/block_devices/sda1", 1024, 512));
    EXPECT_NO_THROW(watcher->onBlockDeviceMounted("/org/freedesktop/UDisks2/block_devices/sda1", "/media/sda1"));
    EXPECT_NO_THROW(watcher->onBlockDeviceLocked("/org/freedesktop/UDisks2/block_devices/sda1"));

    // property change dispatch: block prefix but unknown property → plain emit
    EXPECT_NO_THROW(watcher->onDevicePropertyChangedQVar(
            "/org/freedesktop/UDisks2/block_devices/sda1", "SomeUnknownProperty", QVariant(true)));
    EXPECT_NO_THROW(watcher->onDevicePropertyChangedQDBusVar(
            "/org/freedesktop/UDisks2/block_devices/sda1", "SomeUnknownProperty", QDBusVariant(QVariant(true))));

    // generic attribute / dconfig notifications
    EXPECT_NO_THROW(watcher->onGenAttributeChanged(Application::GenericAttribute::kShowFileSystemTagOnDiskIcon, QVariant(true)));
    EXPECT_NO_THROW(watcher->onGenAttributeChanged(Application::GenericAttribute::kHiddenSystemPartition, QVariant(true)));
    EXPECT_NO_THROW(watcher->onDConfigChanged("org.deepin.dde.file-manager.computer", "hide-block-devices"));
    EXPECT_NO_THROW(watcher->onDConfigChanged("org.deepin.dde.file-manager.computer", "hideMyDirectories"));
    EXPECT_NO_THROW(watcher->onDConfigChanged("org.deepin.dde.file-manager", "dfm.disk.hidden"));

    QTest::qWait(20);
}

TEST_F(UT_ComputerItemWatcherReal, ViewRefresh_RerunsQuery_StillConsistent)
{
    EXPECT_NO_THROW(watcher->onViewRefresh());
    EXPECT_FALSE(watcher->getInitedItems().isEmpty());
}
