// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/base/device/private/defendercontroller.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <dfm-mount/base/ddevicemonitor.h>

#include <QStandardPaths>
#include <QProcess>
#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static constexpr char kBlockDevCase1[] { "/org/freedesktop/UDisks2/block_devices/sdb1" };
static constexpr char kBlockDevCase2[] { "/org/freedesktop/UDisks2/block_devices/loop1" };
static constexpr char kProtocolDevCase1[] { "smb://1.2.3.4/helloworld" };

class UT_DeviceHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_DeviceHelper, CreateDevice)
{
    stub.set_lamda(&DDeviceMonitor::createDeviceById, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_FALSE(DeviceHelper::createDevice("", DeviceType::kAllDevice));
    EXPECT_FALSE(DeviceHelper::createDevice("", DeviceType::kBlockDevice));
    EXPECT_FALSE(DeviceHelper::createDevice("", DeviceType::kProtocolDevice));

    EXPECT_FALSE(DeviceHelper::createDevice(kBlockDevCase1, DeviceType::kAllDevice));
    EXPECT_FALSE(DeviceHelper::createDevice(kBlockDevCase1, DeviceType::kBlockDevice));
    EXPECT_FALSE(DeviceHelper::createDevice(kBlockDevCase1, DeviceType::kProtocolDevice));

    EXPECT_FALSE(DeviceHelper::createDevice(kProtocolDevCase1, DeviceType::kAllDevice));
    EXPECT_FALSE(DeviceHelper::createDevice(kProtocolDevCase1, DeviceType::kBlockDevice));
    EXPECT_FALSE(DeviceHelper::createDevice(kProtocolDevCase1, DeviceType::kProtocolDevice));
}

TEST_F(UT_DeviceHelper, CreateBlockDevice)
{
    stub.set_lamda(DeviceHelper::createDevice, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_FALSE(DeviceHelper::createBlockDevice(""));
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::createBlockDevice(kBlockDevCase1));
    EXPECT_FALSE(DeviceHelper::createBlockDevice(kBlockDevCase2));
}

TEST_F(UT_DeviceHelper, CreateProtocolDevice)
{
    stub.set_lamda(DeviceHelper::createDevice, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_FALSE(DeviceHelper::createProtocolDevice(""));
    EXPECT_FALSE(DeviceHelper::createProtocolDevice(kProtocolDevCase1));
}

TEST_F(UT_DeviceHelper, LoadBlockInfo)
{
    stub.set_lamda(DeviceHelper::createBlockDevice, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::loadBlockInfo(""));

    stub.clear();
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::loadBlockInfo(kBlockDevCase2));
    EXPECT_FALSE(DeviceHelper::loadBlockInfo(kBlockDevCase2).isEmpty());
}

TEST_F(UT_DeviceHelper, LoadProtocolInfo) { }

TEST_F(UT_DeviceHelper, IsMountableBlockDev)
{
    // test isMountableBlockDev(const QString &id, QString &why)
    auto blkPtrStr = static_cast<bool (*)(const BlockDevAutoPtr &, QString &)>(DeviceHelper::isMountableBlockDev);
    stub.set_lamda(blkPtrStr, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(DeviceHelper::createBlockDevice, [] { __DBG_STUB_INVOKE__ return nullptr; });
    QString reason;
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isMountableBlockDev(kBlockDevCase1, reason));
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isMountableBlockDev(kBlockDevCase2, reason));

    // test isMountableBlockDev(const BlockDevAutoPtr &dev, QString &why)
    stub.clear();
    auto loadInfoWithPtr = static_cast<QVariantMap (*)(const BlockDevAutoPtr &)>(DeviceHelper::loadBlockInfo);
    stub.set_lamda(loadInfoWithPtr, [] { __DBG_STUB_INVOKE__ return QVariantMap {}; });
    auto qvmStr = static_cast<bool (*)(const QVariantMap &, QString &)>(DeviceHelper::isMountableBlockDev);
    stub.set_lamda(qvmStr, [] { __DBG_STUB_INVOKE__ return true; });
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isMountableBlockDev(BlockDevAutoPtr(nullptr), reason));
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(BlockDevAutoPtr(nullptr), reason));

    auto loopDev = DeviceHelper::createBlockDevice(kBlockDevCase2);
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isMountableBlockDev(loopDev, reason));
    EXPECT_TRUE(DeviceHelper::isMountableBlockDev(loopDev, reason));

    // test isMountableBlockDev(const QVariantMap &infos, QString &why)
    stub.clear();
    QVariantMap devCase { { "Id", "" },
                          { "HintIgnore", true },
                          { "MountPoint", "/home" },
                          { "HasFileSystem", false },
                          { "IsEncrypted", true } };
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(devCase, reason));
    devCase["Id"] = "1234";
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(devCase, reason));
    devCase["HintIgnore"] = false;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(devCase, reason));
    devCase["MountPoint"] = "";
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(devCase, reason));
    devCase["HasFileSystem"] = true;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(devCase, reason));
    devCase["IsEncrypted"] = false;
    EXPECT_TRUE(DeviceHelper::isMountableBlockDev(devCase, reason));
}

TEST_F(UT_DeviceHelper, IsEjectableBlockDev)
{
    QString reason;
    // test isEjectableBlockDev(const QString &id, QString &why)
    stub.set_lamda(DeviceHelper::createBlockDevice, [] { __DBG_STUB_INVOKE__ return nullptr; });
    auto ptrStr = static_cast<bool (*)(const BlockDevAutoPtr &, QString &)>(DeviceHelper::isEjectableBlockDev);
    stub.set_lamda(ptrStr, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isEjectableBlockDev("", reason));
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isEjectableBlockDev(kBlockDevCase2, reason));
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(kBlockDevCase2, reason));

    // test isEjectableBlockDev(const BlockDevAutoPtr &dev, QString &why)
    stub.clear();
    auto qvmStr = static_cast<bool (*)(const QVariantMap &, QString &)>(DeviceHelper::isEjectableBlockDev);
    stub.set_lamda(qvmStr, [] { __DBG_STUB_INVOKE__ return true; });
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isEjectableBlockDev(BlockDevAutoPtr(nullptr), reason));
    EXPECT_FALSE(DeviceHelper::isEjectableBlockDev(BlockDevAutoPtr(nullptr), reason));

    auto loopDev = DeviceHelper::createBlockDevice(kBlockDevCase2);
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::isEjectableBlockDev(loopDev, reason));
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(loopDev, reason));

    // test isEjectableBlockDev(const QVariantMap &infos, QString &why)
    stub.clear();
    QVariantMap devCase { { "Removable", true },
                          { "Optical", true },
                          { "Ejectable", true } };
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(devCase, reason));
    devCase["Removable"] = false;
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(devCase, reason));
    devCase["Optical"] = false;
    EXPECT_FALSE(DeviceHelper::isEjectableBlockDev(devCase, reason));
    EXPECT_EQ("device is not removable or is not ejectable optical item", reason);
}

TEST_F(UT_DeviceHelper, AskForStopScanning)
{
    bool isScanning = false;
    auto isScannint_QUrl = static_cast<bool (DefenderController::*)(const QUrl &)>(&DefenderController::isScanning);
    stub.set_lamda(isScannint_QUrl, [&] { __DBG_STUB_INVOKE__ return isScanning; });
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::askForStopScanning(QUrl()));
    EXPECT_TRUE(DeviceHelper::askForStopScanning(QUrl()));

    isScanning = true;

    DDialog dlg;
    QDialog::DialogCode execCode = QDialog::Accepted;
    bool scanningStopped = true;
    stub.set_lamda(&DialogManager::showQueryScanningDialog, [&] { __DBG_STUB_INVOKE__ return &dlg; });
    stub.set_lamda(VADDR(DDialog, exec), [&] { __DBG_STUB_INVOKE__ return execCode; });
    auto stopScanning_QUrl = static_cast<bool (DefenderController::*)(const QUrl &)>(&DefenderController::stopScanning);
    stub.set_lamda(stopScanning_QUrl, [&] { __DBG_STUB_INVOKE__ return scanningStopped; });

    EXPECT_TRUE(DeviceHelper::askForStopScanning(QUrl::fromLocalFile("/home")));
    scanningStopped = false;
    EXPECT_FALSE(DeviceHelper::askForStopScanning(QUrl::fromLocalFile("/home")));

    execCode = QDialog::Rejected;
    EXPECT_FALSE(DeviceHelper::askForStopScanning(QUrl::fromLocalFile("/home")));
}

TEST_F(UT_DeviceHelper, OpenFileManagerToDevice)
{
    QString dfmPath = "";
    bool dfmStarted = false;
    stub.set_lamda(QStandardPaths::findExecutable, [&] { __DBG_STUB_INVOKE__ return dfmPath; });
    auto startDetached_QString_QStringList = static_cast<bool (*)(const QString &, const QStringList &)>(QProcess::startDetached);
    stub.set_lamda(startDetached_QString_QStringList, [&] { __DBG_STUB_INVOKE__ dfmStarted = true;  return true; });

    bool showFolder_invoked = false;
#ifdef COMPILE_ON_V23
    auto showFolder_QUrl_QString = qOverload<const QUrl &, const QString &>(&DDesktopServices::showFolder);
#else
    auto showFolder_QUrl_QString = qOverload<QUrl, const QString &>(&DDesktopServices::showFolder);
#endif
    stub.set_lamda(showFolder_QUrl_QString, [&] { __DBG_STUB_INVOKE__ showFolder_invoked = true; return true; });

    // test dfm bin not found
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::openFileManagerToDevice("", ""));
    EXPECT_TRUE(showFolder_invoked);

    // test dfm bin found
    dfmPath = "/usr/bin/dde-file-manager";
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::openFileManagerToDevice("", ""));
    EXPECT_TRUE(dfmStarted);
}

TEST_F(UT_DeviceHelper, CastFromDFMMountProperty)
{
    using namespace GlobalServerDefines;
    using namespace DFMMOUNT;
    EXPECT_TRUE(DeviceHelper::castFromDFMMountProperty(Property::kBlockConfiguration).isEmpty());
    EXPECT_EQ(DeviceProperty::kSizeTotal, DeviceHelper::castFromDFMMountProperty(Property::kBlockSize));
    EXPECT_EQ(DeviceProperty::kMedia, DeviceHelper::castFromDFMMountProperty(Property::kDriveMedia));
}

TEST_F(UT_DeviceHelper, PersistentOpticalInfo)
{
    bool setValue_invoked = false;
    bool sync_invoked = false;

    auto setValue_QString_QString_QVariant = static_cast<void (Settings::*)(const QString &, const QString &, const QVariant &)>(&Settings::setValue);
    stub.set_lamda(setValue_QString_QString_QVariant, [&] { __DBG_STUB_INVOKE__ setValue_invoked = true; });
    stub.set_lamda(&Settings::sync, [&] { __DBG_STUB_INVOKE__ sync_invoked = true; return true; });

    EXPECT_NO_FATAL_FAILURE(DeviceHelper::persistentOpticalInfo({}));
    EXPECT_TRUE(setValue_invoked);
    EXPECT_TRUE(sync_invoked);
}

TEST_F(UT_DeviceHelper, ReadOpticalInfo)
{
    static constexpr char kBurnTotalSize[] { "BurnTotalSize" };
    static constexpr char kBurnUsedSize[] { "BurnUsedSize" };
    static constexpr char kBurnMediaType[] { "BurnMediaType" };
    static constexpr char kBurnWriteSpeed[] { "BurnWriteSpeed" };

    QVariantMap persistentedInfo { { kBurnTotalSize, 1024 },
                                   { kBurnUsedSize, 512 },
                                   { kBurnMediaType, "cd_r" },
                                   { kBurnWriteSpeed, QStringList { "123", "456" } } };
    bool keys_invoked = false, value_invoked = false;

    stub.set_lamda(&Settings::keys, [&] { __DBG_STUB_INVOKE__
                keys_invoked = true;
                return QSet<QString> { "sr0", "sr1" }; });
    auto value_QString_QString_QVariant = static_cast<QVariant (Settings::*)(const QString &, const QString &, const QVariant &) const>(&Settings::value);
    stub.set_lamda(value_QString_QString_QVariant, [&] { __DBG_STUB_INVOKE__
                value_invoked = true;
                return QVariant::fromValue<QVariantMap>(persistentedInfo); });

    QVariantMap retValue;
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::readOpticalInfo(retValue));
    retValue.insert("Device", "/dev/sr0");
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::readOpticalInfo(retValue));
    EXPECT_TRUE(keys_invoked && value_invoked);
    EXPECT_TRUE(retValue.contains("SizeTotal"));
    EXPECT_TRUE(retValue.contains("SizeUsed"));
    EXPECT_EQ(retValue.value("SizeUsed").toInt(), 512);
    EXPECT_EQ(retValue.value("SizeTotal").toInt(), 1024);
}

TEST_F(UT_DeviceHelper, CheckNetworkConnection)
{
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::checkNetworkConnection(""));
    EXPECT_TRUE(DeviceHelper::checkNetworkConnection(""));

    bool checkNetworkConnection_invoked = false;
    typedef bool (NetworkUtils::*CheckFunc)(const QString &, const QString &, int );
        auto check = static_cast<CheckFunc>(&NetworkUtils::checkNetConnection);
        stub.set_lamda(check, [&]() { __DBG_STUB_INVOKE__ checkNetworkConnection_invoked = true; return true; });
    EXPECT_TRUE(DeviceHelper::checkNetworkConnection("smb://1.2.3.4/hello"));
    EXPECT_TRUE(DeviceHelper::checkNetworkConnection("ftp://1.2.3.4"));
    EXPECT_TRUE(DeviceHelper::checkNetworkConnection("sftp://1.2.3.4"));
    EXPECT_TRUE(DeviceHelper::checkNetworkConnection("file:///run/user/1000/gvfs/smb-share:host=1.2.3.4,share=share"));
}

TEST_F(UT_DeviceHelper, MakeFakeProtocolInfo)
{
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::makeFakeProtocolInfo(""));
    EXPECT_FALSE(DeviceHelper::makeFakeProtocolInfo("").isEmpty());
    EXPECT_TRUE(DeviceHelper::makeFakeProtocolInfo("").value("fake").toBool());
    EXPECT_EQ(QObject::tr("Unknown"), DeviceHelper::makeFakeProtocolInfo("").value("DisplayName").toString());

    QString id = "file:///run/user/1000/gvfs/smb-share:server=1.2.3.4,share=share";
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::makeFakeProtocolInfo(id));
    EXPECT_EQ(QObject::tr("%1 on %2").arg("share").arg("1.2.3.4"), DeviceHelper::makeFakeProtocolInfo(id).value("DisplayName").toString());
    id = "file:///run/user/1000/gvfs/smb-share:host=1.2.3.4,share=share";
    EXPECT_EQ(QObject::tr("Unknown"), DeviceHelper::makeFakeProtocolInfo(id).value("DisplayName").toString());

    id = "file:///run/user/1000/gvfs/sftp:host=1.2.3.4";
    EXPECT_NO_FATAL_FAILURE(DeviceHelper::makeFakeProtocolInfo(id));
    EXPECT_EQ("1.2.3.4", DeviceHelper::makeFakeProtocolInfo(id).value("DisplayName").toString());
}
