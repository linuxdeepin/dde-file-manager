// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/base/device/private/devicewatcher.h>
#include <dfm-base/base/device/private/devicewatcher_p.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-mount/dblockmonitor.h>
#include <dfm-burn/dopticaldiscmanager.h>
#include <dfm-burn/dopticaldiscinfo.h>

#include <QStorageInfo>
#include <QHash>
#include <QVariantMap>
#include <QtConcurrent>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_DeviceWatcher : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        watcher = new DeviceWatcher();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

private:
    stub_ext::StubExt stub;
    DeviceWatcher *watcher { nullptr };
};

TEST_F(UT_DeviceWatcher, GetDevInfo)
{
    // test invalid inputs
    EXPECT_TRUE(watcher->getDevInfo("", DFMMOUNT::DeviceType::kAllDevice, false).isEmpty());

    auto loadBlkInfo_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlkInfo_QString, [] { __DBG_STUB_INVOKE__ return QVariantMap { { "test", true } }; });
    EXPECT_TRUE(watcher->getDevInfo("testId", DFMMOUNT::DeviceType::kBlockDevice, true).contains("test"));
    EXPECT_TRUE(watcher->getDevInfo("testId", DFMMOUNT::DeviceType::kBlockDevice, true).value("test").toBool());
    EXPECT_TRUE(watcher->d->allBlockInfos.contains("testId"));

    auto loadProto_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadProtocolInfo);
    stub.set_lamda(loadProto_QString, [] { __DBG_STUB_INVOKE__ return QVariantMap { { "test", true } }; });
    EXPECT_TRUE(watcher->getDevInfo("testId", DFMMOUNT::DeviceType::kProtocolDevice, true).contains("test"));
    EXPECT_TRUE(watcher->getDevInfo("testId", DFMMOUNT::DeviceType::kProtocolDevice, true).value("test").toBool());
    EXPECT_TRUE(watcher->d->allProtocolInfos.contains("testId"));
}

TEST_F(UT_DeviceWatcher, GetDevIDs)
{
    EXPECT_TRUE(watcher->getDevIds(DFMMOUNT::DeviceType::kAllDevice).isEmpty());
    EXPECT_TRUE(watcher->getDevIds(DFMMOUNT::DeviceType::kBlockDevice).isEmpty());
    EXPECT_TRUE(watcher->getDevIds(DFMMOUNT::DeviceType::kProtocolDevice).isEmpty());
}

TEST_F(UT_DeviceWatcher, GetSiblings)
{
    // test invalid inputs
    EXPECT_TRUE(watcher->getSiblings("").isEmpty());
    EXPECT_TRUE(watcher->getSiblings("smb://1.2.3.4/hello").isEmpty());

    auto loadBlkInfo_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlkInfo_QString, [] { __DBG_STUB_INVOKE__ return QVariantMap { { "test", true } }; });
    stub.set_lamda(&DFMMOUNT::DBlockMonitor::resolveDeviceFromDrive, [] { __DBG_STUB_INVOKE__ return QStringList {}; });
    EXPECT_TRUE(watcher->getSiblings("/org/freedesktop/UDisks2/block_devices/loop1").isEmpty());
}

TEST_F(UT_DeviceWatcher, StartPollingUsage)
{
    stub.set_lamda(&DeviceWatcherPrivate::queryUsageAsync, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(watcher->startPollingUsage());
    EXPECT_TRUE(watcher->d->pollingTimer.isActive());
    EXPECT_NO_FATAL_FAILURE(watcher->startPollingUsage());
}

TEST_F(UT_DeviceWatcher, StopPollingUsage)
{
    EXPECT_NO_FATAL_FAILURE(watcher->stopPollingUsage());
    EXPECT_FALSE(watcher->d->pollingTimer.isActive());
}

TEST_F(UT_DeviceWatcher, StartStopWatch)
{
    EXPECT_NO_FATAL_FAILURE(watcher->startWatch());
    EXPECT_TRUE(watcher->d->isWatching);
    EXPECT_NO_FATAL_FAILURE(watcher->startWatch());

    EXPECT_NO_FATAL_FAILURE(watcher->stopWatch());
    EXPECT_FALSE(watcher->d->isWatching);
    EXPECT_TRUE(watcher->d->connections.isEmpty());
}
// todo xushitong, UT broken the ci process
//TEST_F(UT_DeviceWatcher, InitDevDatas)
//{
//    EXPECT_NO_FATAL_FAILURE(watcher->initDevDatas());
//    EXPECT_FALSE(watcher->d->allBlockInfos.isEmpty());
//}

TEST_F(UT_DeviceWatcher, QueryOpticalDevUsage)
{
    QVariantMap blkInfo;
    bool loadBlkInfo_invoked = false;
    auto loadBlkInfo_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlkInfo_QString, [&] { __DBG_STUB_INVOKE__ loadBlkInfo_invoked = true; return blkInfo; });

    // test invalid inputs
    EXPECT_NO_FATAL_FAILURE(watcher->queryOpticalDevUsage(""));
    EXPECT_TRUE(loadBlkInfo_invoked);
    loadBlkInfo_invoked = false;

    blkInfo.insert("Id", "test");
    stub.set_lamda(&dfmburn::DOpticalDiscManager::createOpticalInfo, [&] { __DBG_STUB_INVOKE__ return new DFMBURN::DOpticalDiscInfo; });
    stub.set_lamda(&DFMBURN::DOpticalDiscInfo::totalSize, [] { __DBG_STUB_INVOKE__ return 102400; });
    stub.set_lamda(&DFMBURN::DOpticalDiscInfo::usedSize, [] { __DBG_STUB_INVOKE__ return 51200; });
    stub.set_lamda(&DFMBURN::DOpticalDiscInfo::mediaType, [] { __DBG_STUB_INVOKE__ return DFMBURN::MediaType::kCD_R; });
    stub.set_lamda(&DFMBURN::DOpticalDiscInfo::writeSpeed, [] { __DBG_STUB_INVOKE__ return QStringList { "1", "2" }; });
    bool persistentData_invoked = false;
    bool opticalReload_invoked = false;
    stub.set_lamda(DeviceHelper::persistentOpticalInfo, [&] { __DBG_STUB_INVOKE__ persistentData_invoked = true; });
    stub.set_lamda(&DeviceProxyManager::reloadOpticalInfo, [&] { __DBG_STUB_INVOKE__ opticalReload_invoked = true; });

    EXPECT_NO_FATAL_FAILURE(watcher->queryOpticalDevUsage("testId"));
    EXPECT_TRUE(persistentData_invoked);
    EXPECT_TRUE(opticalReload_invoked);
}

TEST_F(UT_DeviceWatcher, OnBlkDevAdded)
{
    bool devCreate_invokde = false, loadBlock_invoked = false, autoMount_invoked = false;
    stub.set_lamda(DeviceHelper::createBlockDevice, [&] {
        __DBG_STUB_INVOKE__
        devCreate_invokde = true;
        return QSharedPointer<DFMMOUNT::DBlockDevice>(nullptr);
    });
    auto loadBlockInfo_BlkPtr = static_cast<QVariantMap (*)(const BlockDevAutoPtr &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlockInfo_BlkPtr, [&] {
        __DBG_STUB_INVOKE__
        loadBlock_invoked = true;
        return QVariantMap { { "Id", "test" } };
    });
    stub.set_lamda(&DeviceManager::doAutoMount, [&] { __DBG_STUB_INVOKE__ autoMount_invoked = true; });

    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevAdded(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevAdded("test"));
    EXPECT_TRUE((devCreate_invokde && loadBlock_invoked && autoMount_invoked));
}

TEST_F(UT_DeviceWatcher, OnBlkDevRemoved)
{
    watcher->d->allBlockInfos.insert("testId", QVariantMap());
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevRemoved("testId"));
    EXPECT_TRUE(watcher->d->allBlockInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnBlkDevMounted)
{
    bool queryItem_invoked = false;
    stub.set_lamda(&DeviceWatcherPrivate::queryUsageOfItem, [&] { __DBG_STUB_INVOKE__ queryItem_invoked = true; });

    // test invalid inputs
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevMounted("", ""));

    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevMounted("/org/freedesktop/UDisks2/block_devices/sdb1", "/home"));
    int maxWait = 3;
    do {
        QThread::msleep(50);
    } while (!queryItem_invoked && maxWait--);
    EXPECT_TRUE(queryItem_invoked);
}

TEST_F(UT_DeviceWatcher, OnBlkDevUnmounted)
{
    watcher->d->allBlockInfos.insert("testId", QVariantMap());
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevUnmounted("testId"));
    EXPECT_FALSE(watcher->d->allBlockInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnBlkDevLocked)
{
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevLocked(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevLocked("/org/freedesktop/UDisks2/block_devices/sdb1"));
}

TEST_F(UT_DeviceWatcher, OnBlkDevUnlocked)
{
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevUnlocked("", ""));
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevUnlocked("/org/freedesktop/UDisks2/block_devices/sdb1", ""));
}

TEST_F(UT_DeviceWatcher, OnBlkDevFsAdded)
{
    bool blockInfo_invoked = false;
    auto loadBlkInfo_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlkInfo_QString, [&] { __DBG_STUB_INVOKE__ blockInfo_invoked = true; return QVariantMap { { "test", true } }; });

    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevFsAdded(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevFsAdded("/org/freedesktop/UDisks2/block_devices/sdb1"));
    EXPECT_TRUE(blockInfo_invoked);
}

TEST_F(UT_DeviceWatcher, OnBlkDevFsRemoved)
{
    bool blockInfo_invoked = false;
    auto loadBlkInfo_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadBlkInfo_QString, [&] { __DBG_STUB_INVOKE__ blockInfo_invoked = true; return QVariantMap { { "test", true } }; });

    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevFsRemoved(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevFsRemoved("/org/freedesktop/UDisks2/block_devices/sdb1"));

    EXPECT_FALSE(watcher->d->allBlockInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnBlkDevPropertiesChanged)
{
    QMap<DFMMOUNT::Property, QVariant> changes { { DFMMOUNT::Property::kBlockIDLabel, "test" },
                                                 { DFMMOUNT::Property::kFileSystemMountPoint, QStringList { "/home" } },
                                                 { DFMMOUNT::Property::kDriveOptical, false } };
    EXPECT_NO_FATAL_FAILURE(watcher->onBlkDevPropertiesChanged("testId", changes));
    EXPECT_FALSE(watcher->d->allBlockInfos.isEmpty());
    EXPECT_TRUE(watcher->d->allBlockInfos.contains("testId"));
    auto info = watcher->d->allBlockInfos.value("testId");
    EXPECT_FALSE(info.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnProtoDevAdded)
{
    bool autoMount_invoked = false;
    stub.set_lamda(&DeviceManager::doAutoMount, [&] { __DBG_STUB_INVOKE__ autoMount_invoked = true; });

    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevAdded(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevAdded("smb://1.2.3.4/hello"));
    EXPECT_TRUE(autoMount_invoked);
    EXPECT_FALSE(watcher->d->allProtocolInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnProtoDevRemoved)
{
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevRemoved(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevRemoved("smb://1.2.3.4/Hello"));
    EXPECT_TRUE(watcher->d->allProtocolInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnProtoDevMounted)
{
    auto loadProto_QString = static_cast<QVariantMap (*)(const QString &)>(DeviceHelper::loadProtocolInfo);
    stub.set_lamda(loadProto_QString, [] { __DBG_STUB_INVOKE__ return QVariantMap { { "test", true } }; });
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevMounted("", ""));
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevMounted("smb://1.2.3.4/hello", "/run/user/1000/gvfs/smb-share:server=1.2.3.4,share=hello"));
    EXPECT_FALSE(watcher->d->allProtocolInfos.isEmpty());
}

TEST_F(UT_DeviceWatcher, OnProtoDevUnmounted)
{
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevUnmounted(""));
    EXPECT_NO_FATAL_FAILURE(watcher->onProtoDevUnmounted("smb://1.2.3.4/Hello"));
    EXPECT_TRUE(watcher->d->allProtocolInfos.isEmpty());
}

class UT_DeviceWatcherPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        watcher = new DeviceWatcher();
        pd = watcher->d.data();

        pd->allBlockInfos.insert("/org/freedesktop/UDisks2/block_devices/loop1", {});
        pd->allProtocolInfos.insert("smb://1.2.3.4/hello", {});
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

private:
    stub_ext::StubExt stub;
    DeviceWatcher *watcher { nullptr };
    DeviceWatcherPrivate *pd { nullptr };
};

// using RunTarget = std::function<void()>;
// void stub_run(RunTarget func)
//{
//     __DBG_STUB_INVOKE__
//     if (func)
//         func();
// }
TEST_F(UT_DeviceWatcherPrivate, QueryUsageAsync)
{
    bool query_invoked = false;
    stub.set_lamda(&DeviceWatcherPrivate::queryUsageOfItem, [&] { __DBG_STUB_INVOKE__ query_invoked = true; });

    //    typedef QFuture<void> (*RunType)(RunTarget);
    //    auto run = static_cast<RunType>(QtConcurrent::run);
    //    stub.set(run, stub_run);

    EXPECT_NO_FATAL_FAILURE(pd->queryUsageAsync());
    int maxWait = 3;
    do {   // wait QtConcurrent::run finished
        QThread::msleep(50);
    } while (!query_invoked && maxWait--);
    EXPECT_TRUE(query_invoked);
}

TEST_F(UT_DeviceWatcherPrivate, UpdateStorage)
{
    EXPECT_NO_FATAL_FAILURE(pd->updateStorage("/org/freedesktop/UDisks2/block_devices/loop1", 100, 50));
    EXPECT_NO_FATAL_FAILURE(pd->updateStorage("smb://1.2.3.4/hello", 102400, 51200));

    auto blkDevInfo = pd->allBlockInfos.value("/org/freedesktop/UDisks2/block_devices/loop1");
    EXPECT_TRUE(blkDevInfo.value("SizeTotal").toInt() == 100);
    EXPECT_TRUE(blkDevInfo.value("SizeUsed").toInt() == 50);

    auto protoDevInfo = pd->allProtocolInfos.value("smb://1.2.3.4/hello");
    EXPECT_TRUE(protoDevInfo.value("SizeTotal").toInt() == 102400);
    EXPECT_TRUE(protoDevInfo.value("SizeUsed").toInt() == 51200);
}

TEST_F(UT_DeviceWatcherPrivate, QueryUsageOfItem)
{
    bool blkQueried = false, protoQueried = false;
    DevStorage newStorage { 102400, 1024, 102400 - 1024 };
    stub.set_lamda(&DeviceWatcherPrivate::queryUsageOfBlock, [&] { __DBG_STUB_INVOKE__ blkQueried = true; return newStorage; });
    stub.set_lamda(&DeviceWatcherPrivate::queryUsageOfProtocol, [&] { __DBG_STUB_INVOKE__ protoQueried = true; return newStorage; });

    // test invalid inputs
    EXPECT_NO_FATAL_FAILURE(pd->queryUsageOfItem({}, DFMMOUNT::DeviceType::kBlockDevice));
    QVariantMap testInfo { { "MountPoint", "/home" } };
    EXPECT_NO_FATAL_FAILURE(pd->queryUsageOfItem(testInfo, DFMMOUNT::DeviceType::kAllDevice));

    EXPECT_NO_FATAL_FAILURE(pd->queryUsageOfItem(testInfo, DFMMOUNT::DeviceType::kBlockDevice));
    EXPECT_NO_FATAL_FAILURE(pd->queryUsageOfItem(testInfo, DFMMOUNT::DeviceType::kProtocolDevice));
    EXPECT_TRUE(blkQueried);
    EXPECT_TRUE(protoQueried);
}

TEST_F(UT_DeviceWatcherPrivate, QueryUsageOfBlock)
{
    stub.set_lamda(DeviceHelper::readOpticalInfo, [](QVariantMap &info) {
        __DBG_STUB_INVOKE__
        info["SizeTotal"] = 102400;
        info["SizeFree"] = 51200;
        info["SizeUsed"] = 51200;
    });
    stub.set_lamda(&QStorageInfo::bytesAvailable, [] { __DBG_STUB_INVOKE__ return 51200; });

    QVariantMap devInfo { {} };
    EXPECT_FALSE(pd->queryUsageOfBlock(devInfo).isValid());

    devInfo["MountPoint"] = "/home";
    devInfo["OpticalDrive"] = true;
    EXPECT_TRUE(pd->queryUsageOfBlock(devInfo).isValid());

    DevStorage opticalInfo { 102400, 51200, 51200 };
    DevStorage queriedInfo = pd->queryUsageOfBlock(devInfo);
    EXPECT_EQ(opticalInfo.total, queriedInfo.total);
    EXPECT_EQ(opticalInfo.avai, queriedInfo.avai);
    EXPECT_EQ(opticalInfo.used, queriedInfo.used);

    devInfo.remove("OpticalDrive");
    devInfo.insert("SizeTotal", 51200);
    DevStorage blockInfo { 51200, 51200, 0 };
    queriedInfo = pd->queryUsageOfBlock(devInfo);
    EXPECT_EQ(blockInfo.total, queriedInfo.total);
    EXPECT_EQ(blockInfo.avai, queriedInfo.avai);
    EXPECT_EQ(blockInfo.used, queriedInfo.used);
}

TEST_F(UT_DeviceWatcherPrivate, QueryUsageOfProtocol)
{
    QVariantMap devInfo {};
    EXPECT_FALSE(pd->queryUsageOfProtocol(devInfo).isValid());

    devInfo["MountPoint"] = "/home";
    EXPECT_FALSE(pd->queryUsageOfProtocol(devInfo).isValid());

    devInfo["Id"] = "smb://1.2.3.4/hello";
    int testPtr = 0;
    stub.set_lamda(DeviceHelper::createProtocolDevice, [&] {
        __DBG_STUB_INVOKE__
        return testPtr == 0
                ? nullptr
                : QSharedPointer<DFMMOUNT::DProtocolDevice>(reinterpret_cast<DFMMOUNT::DProtocolDevice *>(&testPtr));
    });
    EXPECT_FALSE(pd->queryUsageOfProtocol(devInfo).isValid());

    // cannot get a DProtocolDevice instance.
    //    testPtr = 1;
    //    stub.set_lamda(&DFMMOUNT::DDevice::sizeTotal, [] { __DBG_STUB_INVOKE__ return 102400; });
    //    stub.set_lamda(&DFMMOUNT::DDevice::sizeFree, [] { __DBG_STUB_INVOKE__ return 51200; });
    //    stub.set_lamda(&DFMMOUNT::DDevice::sizeUsage, [] { __DBG_STUB_INVOKE__ return 51200; });

    //    DevStorage expected { 102400, 51200, 51200 };
    //    DevStorage real = pd->queryUsageOfProtocol(devInfo);
    //    EXPECT_TRUE(real.isValid());
    //    EXPECT_EQ(expected.total, real.total);
    //    EXPECT_EQ(expected.avai, real.avai);
    //    EXPECT_EQ(expected.used, real.used);
}

class UT_DevStorage : public testing::Test
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
    DevStorage ds1 {};
    DevStorage ds2 { 1, 2, 3 };
    DevStorage ds3 { 1, 2, 3 };
    DevStorage ds4 { 2, 2, 2 };
};

TEST_F(UT_DevStorage, Equals)
{
    EXPECT_TRUE(ds2 == ds3);
    EXPECT_FALSE(ds2 == ds4);
}

TEST_F(UT_DevStorage, NotEquals)
{
    EXPECT_TRUE(ds2 != ds4);
    EXPECT_TRUE(ds1 != ds2);
}

TEST_F(UT_DevStorage, IsValid)
{
    EXPECT_TRUE(ds2.isValid());
    EXPECT_TRUE(ds3.isValid());
    EXPECT_TRUE(ds4.isValid());
    EXPECT_FALSE(ds1.isValid());
}
