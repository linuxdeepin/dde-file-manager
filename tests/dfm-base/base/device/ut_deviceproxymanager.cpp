// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/private/deviceproxymanager_p.h>

#include <QDBusAbstractInterface>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_DeviceProxyManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        usingDBus = false;
        stub.set_lamda(&DeviceProxyManagerPrivate::isDBusRuning, [&] { __DBG_STUB_INVOKE__ return usingDBus; });
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    bool usingDBus = false;
};

TEST_F(UT_DeviceProxyManager, GetDBusIface)
{
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getDBusIFace());
}

TEST_F(UT_DeviceProxyManager, GetAllBlockIDs)
{
    stub.set_lamda(&DeviceManager::getAllBlockDevID, [] { __DBG_STUB_INVOKE__ return QStringList(); });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getAllBlockIds());

    usingDBus = true;
    stub.set_lamda(&OrgDeepinFilemanagerServerDeviceManagerInterface::GetBlockDevicesIdList,
                   [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<QStringList> {}; });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getAllBlockIds());
}

TEST_F(UT_DeviceProxyManager, GetAllBlockIDsByUUID)
{
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds,
                   [] { __DBG_STUB_INVOKE__ return QStringList { "a", "b" }; });
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [] { __DBG_STUB_INVOKE__ return QVariantMap { { "IdUUID", "12341234" } }; });

    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getAllBlockIdsByUUID({}));

    EXPECT_TRUE(DevProxyMng->getAllBlockIdsByUUID({ "12341234" }).count() != 0);
    EXPECT_TRUE(DevProxyMng->getAllBlockIdsByUUID({ "12341234" }).contains("a"));
    EXPECT_TRUE(DevProxyMng->getAllBlockIdsByUUID({ "12341234" }).contains("b"));
}

TEST_F(UT_DeviceProxyManager, GetAllProtocolIDs)
{
    stub.set_lamda(&DeviceManager::getAllProtocolDevID, [] { __DBG_STUB_INVOKE__ return QStringList(); });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getAllProtocolIds());

    usingDBus = true;
    stub.set_lamda(&OrgDeepinFilemanagerServerDeviceManagerInterface::GetProtocolDevicesIdList,
                   [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<QStringList> {}; });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->getAllProtocolIds());
}

TEST_F(UT_DeviceProxyManager, QueryBlockInfo)
{
    stub.set_lamda(&DeviceManager::getBlockDevInfo, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryBlockInfo("", false));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryBlockInfo("1234", false));

    usingDBus = true;
    stub.set_lamda(&OrgDeepinFilemanagerServerDeviceManagerInterface::QueryBlockDeviceInfo,
                   [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<QVariantMap> {}; });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryBlockInfo("", false));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryBlockInfo("1234", false));
}

TEST_F(UT_DeviceProxyManager, QueryProtocolInfo)
{
    stub.set_lamda(&DeviceManager::getProtocolDevInfo, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryProtocolInfo("", false));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryProtocolInfo("1234", false));

    usingDBus = true;
    stub.set_lamda(&OrgDeepinFilemanagerServerDeviceManagerInterface::QueryProtocolDeviceInfo,
                   [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<QVariantMap> {}; });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryProtocolInfo("", false));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->queryProtocolInfo("1234", false));
}

TEST_F(UT_DeviceProxyManager, ReloadOpticalInfo)
{
    usingDBus = false;
    stub.set_lamda(&DeviceManager::getBlockDevInfo, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo("1234"));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo("/dev/sr0"));

    usingDBus = true;
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo("1234"));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->reloadOpticalInfo("/dev/sr0"));
}

TEST_F(UT_DeviceProxyManager, InitService)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initConnection, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DeviceProxyManagerPrivate::isDBusRuning, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->initService());
    EXPECT_TRUE(DevProxyMng->initService());
}

TEST_F(UT_DeviceProxyManager, IsDBusRunning)
{
    usingDBus = false;
    EXPECT_FALSE(DevProxyMng->isDBusRuning());

    usingDBus = true;
    EXPECT_TRUE(DevProxyMng->isDBusRuning());
}

TEST_F(UT_DeviceProxyManager, IsFileOfExternalMounts)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initMounts, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfExternalMounts(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfExternalMounts("111"));
}

TEST_F(UT_DeviceProxyManager, IsFileOfProtoclMounts)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initMounts, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfProtocolMounts(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfProtocolMounts("111"));
    EXPECT_FALSE(DevProxyMng->isFileOfProtocolMounts("111"));
}

TEST_F(UT_DeviceProxyManager, IsFileOfExternalBlockMounts)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initMounts, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfExternalBlockMounts(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileOfExternalBlockMounts("111"));
    EXPECT_FALSE(DevProxyMng->isFileOfExternalBlockMounts("111"));
}

TEST_F(UT_DeviceProxyManager, IsFileFromOptical)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initMounts, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileFromOptical(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isFileFromOptical("111"));
    EXPECT_FALSE(DevProxyMng->isFileFromOptical("111"));
}

TEST_F(UT_DeviceProxyManager, IsMptOfDevice)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::initMounts, [] { __DBG_STUB_INVOKE__ });
    QString id;
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isMptOfDevice("", id));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->isMptOfDevice("111", id));
    EXPECT_FALSE(DevProxyMng->isMptOfDevice("111", id));
}

class UT_DeviceProxyManagerPrivate : public testing::Test
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

TEST_F(UT_DeviceProxyManagerPrivate, IsDBusRuninig)
{
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->isDBusRuning());

    stub.set_lamda(&QDBusAbstractInterface::isValid, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(DevProxyMng->d->isDBusRuning());
}

TEST_F(UT_DeviceProxyManagerPrivate, InitConnection)
{
    bool usingDBus = false;
    stub.set_lamda(&DeviceProxyManagerPrivate::isDBusRuning, [&] { __DBG_STUB_INVOKE__ return usingDBus; });
    stub.set_lamda(&DeviceProxyManagerPrivate::connectToDBus, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DeviceProxyManagerPrivate::connectToAPI, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->initConnection());

    Q_EMIT DevProxyMng->d->dbusWatcher->serviceRegistered("");
    Q_EMIT DevProxyMng->d->dbusWatcher->serviceUnregistered("");
}

TEST_F(UT_DeviceProxyManagerPrivate, InitMounts)
{
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds,
                   [] { __DBG_STUB_INVOKE__ return QStringList { "1234", "4567" }; });
    stub.set_lamda(&DeviceProxyManager::getAllProtocolIds,
                   [] { __DBG_STUB_INVOKE__ return QStringList { "3333", "4444" }; });
    QVariantMap devInfo { { "MountPoint", "/home" },
                          { "Removable", true } };
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [=] { __DBG_STUB_INVOKE__ return devInfo; });
    stub.set_lamda(&DeviceProxyManager::queryProtocolInfo,
                   [=] { __DBG_STUB_INVOKE__ return devInfo; });
    stub.set_lamda(&DeviceUtils::isMountPointOfDlnfs,
                   [] { __DBG_STUB_INVOKE__ return false; });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->initMounts());
}

TEST_F(UT_DeviceProxyManagerPrivate, ConnectToDBus)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::disconnCurrentConnections, [] { __DBG_STUB_INVOKE__ });

    DevProxyMng->d->currentConnectionType = DeviceProxyManagerPrivate::kDBusConnecting;
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->connectToDBus());

    DevProxyMng->d->currentConnectionType = DeviceProxyManagerPrivate::kAPIConnecting;
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->connectToDBus());
}

TEST_F(UT_DeviceProxyManagerPrivate, ConnectToAPI)
{
    stub.set_lamda(&DeviceProxyManagerPrivate::disconnCurrentConnections, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DeviceManager::startMonitor, [] { __DBG_STUB_INVOKE__ });

    DevProxyMng->d->currentConnectionType = DeviceProxyManagerPrivate::kDBusConnecting;
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->connectToAPI());

    DevProxyMng->d->currentConnectionType = DeviceProxyManagerPrivate::kAPIConnecting;
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->connectToAPI());
}

TEST_F(UT_DeviceProxyManagerPrivate, DisconnCurrentConnections)
{
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->disconnCurrentConnections());
    EXPECT_EQ(DevProxyMng->d->currentConnectionType, DeviceProxyManagerPrivate::kNoneConnection);
}

TEST_F(UT_DeviceProxyManagerPrivate, AddMounts)
{
    stub.set_lamda(&DeviceUtils::isMountPointOfDlnfs, [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [] { __DBG_STUB_INVOKE__ return QVariantMap { { "Removable", true } }; });

    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->addMounts("", ""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->addMounts("/org/freedesktop/UDisks2/block_devices/sdb1", ""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->addMounts("xxxxxx", ""));
}

TEST_F(UT_DeviceProxyManagerPrivate, RemoveMounts)
{
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->removeMounts(""));
    EXPECT_NO_FATAL_FAILURE(DevProxyMng->d->removeMounts("1234"));
}
