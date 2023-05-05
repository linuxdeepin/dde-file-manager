// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QUrl>
#include <QSet>

#include <gtest/gtest.h>

#include <libmount.h>

DFMBASE_USE_NAMESPACE

class UT_DeviceUtils : public testing::Test
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

TEST_F(UT_DeviceUtils, bug_139803_UseLibmount)
{
    bool useLibMountInterfaces { false };
    stub.set_lamda(&mnt_new_table, [&useLibMountInterfaces] {
        __DBG_STUB_INVOKE__
        useLibMountInterfaces = true;
        libmnt_table *table { NULL };
        return table;
    });
    DeviceUtils::getMountInfo("/dev/sr0");
    EXPECT_TRUE(useLibMountInterfaces);
}

TEST_F(UT_DeviceUtils, GetBlockDeviceId)
{
    EXPECT_EQ("/org/freedesktop/UDisks2/block_devices/sdb1", DeviceUtils::getBlockDeviceId("/dev/sdb1"));
    EXPECT_NE("/org/freedesktop/UDisks2/block_devices/sdb1", DeviceUtils::getBlockDeviceId("/dev/sdb"));
}

TEST_F(UT_DeviceUtils, GetMountInfo)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::getMountInfo("/"));
    EXPECT_FALSE(DeviceUtils::getMountInfo("/", false).isEmpty());
}

TEST_F(UT_DeviceUtils, GetSambaFileUriFromNative)
{
    // invalid url passed
    EXPECT_FALSE(DeviceUtils::getSambaFileUriFromNative(QUrl()).isValid());

    // local file url passed
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::getSambaFileUriFromNative(QUrl::fromLocalFile("/")));
    QUrl homeUrl = QUrl::fromLocalFile("/home");
    EXPECT_EQ(homeUrl, DeviceUtils::getSambaFileUriFromNative(homeUrl));

    // gvfs samba url
    QUrl gvfsSmb = QUrl::fromLocalFile("/run/user/1000/gvfs/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::getSambaFileUriFromNative(gvfsSmb));
    QUrl smbUrl = DeviceUtils::getSambaFileUriFromNative(gvfsSmb);
    EXPECT_EQ(smbUrl.host(), "1.2.3.4");
    EXPECT_EQ(smbUrl.scheme(), "smb");
    EXPECT_EQ(smbUrl.path(), "/v23");

    // cifs samba url
    QUrl cifsSmb = QUrl::fromLocalFile("/media/test/smbmounts/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::getSambaFileUriFromNative(cifsSmb));
    smbUrl = DeviceUtils::getSambaFileUriFromNative(cifsSmb);
    EXPECT_EQ(smbUrl.host(), "1.2.3.4");
    EXPECT_EQ(smbUrl.scheme(), "smb");
    EXPECT_EQ(smbUrl.path(), "/v23");
}

TEST_F(UT_DeviceUtils, ErrMessage)
{
    EXPECT_EQ(DFMMOUNT::Utils::errorMessage(DFMMOUNT::DeviceError::kDaemonErrorCannotGenerateMountPath),
              DeviceUtils::errMessage(DFMMOUNT::DeviceError::kDaemonErrorCannotGenerateMountPath));
}

TEST_F(UT_DeviceUtils, ConvertSuitableDisplayName)
{
    stub.set_lamda(DeviceUtils::nameOfSystemDisk, [] { __DBG_STUB_INVOKE__ return "systemDisk"; });
    stub.set_lamda(DeviceUtils::nameOfEncrypted, [] { __DBG_STUB_INVOKE__ return "encryptedDisk"; });
    stub.set_lamda(DeviceUtils::nameOfOptical, [] { __DBG_STUB_INVOKE__ return "opticalDisc"; });
    stub.set_lamda(DeviceUtils::nameOfDefault, [] { __DBG_STUB_INVOKE__ return "defaultDisk"; });

    QVariantHash devInfo;

    // test system
    devInfo.clear();
    devInfo.insert("HintSystem", true);
    EXPECT_EQ("systemDisk", DeviceUtils::convertSuitableDisplayName(devInfo));

    // test encrypted
    devInfo.clear();
    devInfo.insert("IsEncrypted", true);
    EXPECT_EQ("encryptedDisk", DeviceUtils::convertSuitableDisplayName(devInfo));

    // test optical
    devInfo.clear();
    devInfo.insert("OpticalDrive", true);
    EXPECT_EQ("opticalDisc", DeviceUtils::convertSuitableDisplayName(devInfo));

    // test other
    devInfo.clear();
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::convertSuitableDisplayName(devInfo));
    EXPECT_EQ("defaultDisk", DeviceUtils::convertSuitableDisplayName(devInfo));
}

TEST_F(UT_DeviceUtils, FormatOpticalMediaType)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::formatOpticalMediaType(""));
    EXPECT_EQ("DVD+R", DeviceUtils::formatOpticalMediaType("optical_dvd_plus_r"));
    EXPECT_EQ("BD-ROM", DeviceUtils::formatOpticalMediaType("optical_bd"));

    EXPECT_TRUE(DeviceUtils::formatOpticalMediaType("xxxx").isEmpty());
}

TEST_F(UT_DeviceUtils, IsAutoMountEnable)
{
    stub.set_lamda(&Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isAutoMountEnable());
    EXPECT_TRUE(DeviceUtils::isAutoMountEnable());
}

TEST_F(UT_DeviceUtils, IsAutoMountAndOpenEnable)
{
    stub.set_lamda(&Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isAutoMountAndOpenEnable());
    EXPECT_TRUE(DeviceUtils::isAutoMountAndOpenEnable());
}

TEST_F(UT_DeviceUtils, IsWorkingOpticalDiscDev)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isWorkingOpticalDiscDev(""));
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscDev("/dev/xxx"));

    // fake optical item
    stub.set_lamda(&Settings::keys, [] {
        __DBG_STUB_INVOKE__
        QSet<QString> ret;
        ret.insert("/dev/sr0");
        return ret;
    });

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QVariant> ret;
        ret.insert("Working", true);
        return QVariant::fromValue<QMap<QString, QVariant>>(ret);
    });
    EXPECT_TRUE(DeviceUtils::isWorkingOpticalDiscDev("/dev/sr0"));
}

TEST_F(UT_DeviceUtils, IsWorkingOpticalDiskId)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isWorkingOpticalDiscId(""));
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscId("xxx"));

    // fake optical item
    stub.set_lamda(&Settings::keys, [] {
        __DBG_STUB_INVOKE__
        QSet<QString> ret;
        ret.insert("/dev/sr0");
        return ret;
    });

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QVariant> ret;
        ret.insert("Working", true);
        ret.insert("id", "/dev/sr0");
        return QVariant::fromValue<QMap<QString, QVariant>>(ret);
    });
    EXPECT_TRUE(DeviceUtils::isWorkingOpticalDiscDev("/dev/sr0"));
}

TEST_F(UT_DeviceUtils, IsSamba)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isSamba(QUrl()));

    // smb url test
    QUrl smbUrl("smb://1.2.3.4");
    EXPECT_TRUE(DeviceUtils::isSamba(smbUrl));

    // local file url
    QUrl realLocalUrl = QUrl::fromLocalFile("/");
    EXPECT_FALSE(DeviceUtils::isSamba(realLocalUrl));

    // local smb url
    QUrl gvfsSmb = QUrl::fromLocalFile("/run/user/1000/gvfs/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isSamba(gvfsSmb));
    QUrl cifsSmb = QUrl::fromLocalFile("/media/test/smbmounts/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isSamba(cifsSmb));

    // root gvfs url
    QUrl rootGvfsSmb = QUrl::fromLocalFile("/root/.gvfs/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isSamba(rootGvfsSmb));
}

TEST_F(UT_DeviceUtils, IsFtp)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isFtp(QUrl()));

    // local file
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isFtp(QUrl::fromLocalFile("/")));
    EXPECT_FALSE(DeviceUtils::isFtp(QUrl::fromLocalFile("/")));

    // ftp url test
    QUrl ftp = QUrl::fromLocalFile("/run/user/1000/gvfs/ftp:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isFtp(ftp));

    QUrl rootFtp = QUrl::fromLocalFile("/root/.gvfs/ftp:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isFtp(rootFtp));
}

TEST_F(UT_DeviceUtils, IsSftp)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isSftp(QUrl()));

    // local file
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isSftp(QUrl::fromLocalFile("/")));
    EXPECT_FALSE(DeviceUtils::isSftp(QUrl::fromLocalFile("/")));

    // ftp url test
    QUrl ftp = QUrl::fromLocalFile("/run/user/1000/gvfs/sftp:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isSftp(ftp));

    QUrl rootFtp = QUrl::fromLocalFile("/root/.gvfs/sftp:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::isSftp(rootFtp));
}

TEST_F(UT_DeviceUtils, IsExternalBlock)
{
    stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isExternalBlock(QUrl()));
    EXPECT_TRUE(DeviceUtils::isExternalBlock(QUrl()));
}

TEST_F(UT_DeviceUtils, ParseNetSourceUrl)
{
    // invalid input
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::parseNetSourceUrl(QUrl()));
    EXPECT_FALSE(DeviceUtils::parseNetSourceUrl(QUrl()).isValid());

    // local file
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::parseNetSourceUrl(QUrl::fromLocalFile("/home")));
    EXPECT_FALSE(DeviceUtils::parseNetSourceUrl(QUrl::fromLocalFile("/home")).isValid());

    // gvfs samba url
    QUrl gvfsSmb = QUrl::fromLocalFile("/run/user/1000/gvfs/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::parseNetSourceUrl(gvfsSmb));
    QUrl smbUrl = DeviceUtils::parseNetSourceUrl(gvfsSmb);
    EXPECT_EQ(smbUrl.host(), "1.2.3.4");
    EXPECT_EQ(smbUrl.scheme(), "smb");
    EXPECT_EQ(smbUrl.path(), "/v23");

    // cifs samba url
    QUrl cifsSmb = QUrl::fromLocalFile("/media/test/smbmounts/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::parseNetSourceUrl(cifsSmb));
    smbUrl = DeviceUtils::parseNetSourceUrl(cifsSmb);
    EXPECT_EQ(smbUrl.host(), "1.2.3.4");
    EXPECT_EQ(smbUrl.scheme(), "smb");
    EXPECT_EQ(smbUrl.path(), "/v23");

    QUrl ftpUrl = QUrl::fromLocalFile("/run/user/1000/gvfs/ftp:port=448,host=1.2.3.4");
    EXPECT_EQ("ftp", DeviceUtils::parseNetSourceUrl(ftpUrl).scheme());
    EXPECT_EQ("1.2.3.4", DeviceUtils::parseNetSourceUrl(ftpUrl).host());
}

TEST_F(UT_DeviceUtils, ParseSmbInfo)
{
    QString host, share, port;

    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::parseSmbInfo("", host, share));
    EXPECT_FALSE(DeviceUtils::parseSmbInfo("", host, share));

    // local file inputs
    EXPECT_FALSE(DeviceUtils::parseSmbInfo("/home", host, share));

    // gvfs samba url
    QUrl gvfsSmb = QUrl::fromLocalFile("/run/user/1000/gvfs/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::parseSmbInfo(gvfsSmb.path(), host, share, &port));
    EXPECT_EQ("1.2.3.4", host);
    EXPECT_EQ("v23", share);
    EXPECT_EQ("448", port);

    // cifs samba url
    QUrl cifsSmb = QUrl::fromLocalFile("/media/test/smbmounts/smb-share:port=448,server=1.2.3.4,share=v23");
    EXPECT_TRUE(DeviceUtils::parseSmbInfo(cifsSmb.path(), host, share, &port));
    EXPECT_EQ("1.2.3.4", host);
    EXPECT_EQ("v23", share);
    EXPECT_EQ("448", port);
}

TEST_F(UT_DeviceUtils, FstabBindInfo)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::fstabBindInfo());
}

TEST_F(UT_DeviceUtils, NameOfSystemDisk)
{
    // stub for AliasGroupName-AliasItemName
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        QList<QVariant> alias;
        QVariantMap aliasItem { { "Items", "Datas" },
                                { "alias", "Datas" },
                                { "name", "Datas" },
                                { "uuid", "12345" } };
        alias.append(aliasItem);

        aliasItem = { { "Items", "System" },
                      { "alias", "SystemDisk" },
                      { "name", "SystemDisk" },
                      { "uuid", "34567" } };
        alias.append(aliasItem);
        return QVariant::fromValue<QList<QVariant>>(alias);
    });

    stub.set_lamda(&DeviceUtils::nameOfDefault, [] { __DBG_STUB_INVOKE__ return "Default"; });

    QVariantMap diskInfo { { "IdUUID", "34567" },
                           { "MountPoint", "/" },
                           { "IdLabel", "_dde_data" } };

    EXPECT_EQ("SystemDisk", DeviceUtils::nameOfSystemDisk(diskInfo));

    diskInfo.remove("IdUUID");
    EXPECT_EQ(QObject::tr("System Disk"), DeviceUtils::nameOfSystemDisk(diskInfo));

    diskInfo.remove("MountPoint");
    EXPECT_EQ(QObject::tr("Data Disk"), DeviceUtils::nameOfSystemDisk(diskInfo));

    diskInfo.clear();
    EXPECT_EQ("Default", DeviceUtils::nameOfSystemDisk(diskInfo));
}

TEST_F(UT_DeviceUtils, NameOfOptical)
{
    // invliad input
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfOptical({}));

    QVariantMap discInfo { { "Optical", true },
                           { "OpticalBlank", true },
                           { "Media", "" } };

    stub.set_lamda(&DeviceUtils::nameOfDefault, [] { __DBG_STUB_INVOKE__ return "Default"; });

    // ===== drive loaded
    //      blank disc
    EXPECT_EQ(QObject::tr("Blank %1 Disc").arg(QObject::tr("Unknown")), DeviceUtils::nameOfOptical(discInfo));

    //      non blank disc
    discInfo["OpticalBlank"] = false;
    EXPECT_EQ("Default", DeviceUtils::nameOfOptical(discInfo));
    // ===== drive loaded end

    // ===== NO DISC IN
    discInfo["Optical"] = false;
    discInfo["MediaCompatibility"] = { "optical_cd_r" };
    EXPECT_EQ(QObject::tr("%1 Drive").arg("CD-R"), DeviceUtils::nameOfOptical(discInfo));

    discInfo.remove("MediaCompatibility");
    EXPECT_EQ("Default", DeviceUtils::nameOfOptical(discInfo));
    // ===== NO DISC IN END
}

TEST_F(UT_DeviceUtils, NameOpEncrypted)
{
    stub.set_lamda(&DeviceUtils::nameOfDefault, [] { __DBG_STUB_INVOKE__ return "Default"; });
    stub.set_lamda(&DeviceUtils::nameOfSize, [] { __DBG_STUB_INVOKE__ return "1B"; });

    // invalid input
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfEncrypted({}));

    QVariantMap diskInfo { { "CleartextDevice", "123123123" },
                           { "ClearBlockDeviceInfo", QVariantMap { { "test", "test" } } } };

    EXPECT_EQ("Default", DeviceUtils::nameOfEncrypted(diskInfo));

    diskInfo.clear();
    EXPECT_EQ(QObject::tr("%1 Encrypted").arg("1B"), DeviceUtils::nameOfEncrypted(diskInfo));
}

TEST_F(UT_DeviceUtils, NameOfDefault)
{
    stub.set_lamda(&DeviceUtils::nameOfSize, [] { __DBG_STUB_INVOKE__ return "1B"; });
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfDefault("", 0));
    EXPECT_EQ(QObject::tr("%1 Volume").arg("1B"), DeviceUtils::nameOfDefault("", 0));

    // has label
    EXPECT_EQ("Test", DeviceUtils::nameOfDefault("Test", 0));
}

TEST_F(UT_DeviceUtils, NameOfSize)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfSize(-999999));
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfSize(0));

    quint64 base = 1;
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfSize(base << 128));
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::nameOfSize(base << 63));

    EXPECT_EQ(QString("1.0 B"), DeviceUtils::nameOfSize(1));
    EXPECT_EQ("1.0 KB", DeviceUtils::nameOfSize(base << 10));
    EXPECT_EQ("1.0 MB", DeviceUtils::nameOfSize(base << 20));
    EXPECT_EQ("1.0 TB", DeviceUtils::nameOfSize(base << 40));
    EXPECT_EQ("1024.0 TB", DeviceUtils::nameOfSize(base << 50));

    EXPECT_EQ("1.5 GB", DeviceUtils::nameOfSize((base << 30) + (base << 29)));
    EXPECT_EQ("1.8 GB", DeviceUtils::nameOfSize((base << 30) + (base << 29) + (base << 28)));

    EXPECT_EQ("1.0 KB", DeviceUtils::nameOfSize((base << 10) + (base << 2)));
    EXPECT_EQ("1.6 KB", DeviceUtils::nameOfSize(1588));   // 1.55
    EXPECT_EQ("1.5 KB", DeviceUtils::nameOfSize(1576));   // 1.54
}

TEST_F(UT_DeviceUtils, CheckDiskEncrypted)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::checkDiskEncrypted());
}

TEST_F(UT_DeviceUtils, EncryptedDisks)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::encryptedDisks());
}

TEST_F(UT_DeviceUtils, IsSubpathOfDlnfs)
{
    stub.set_lamda(DeviceUtils::findDlnfsPath, [](const QString &, DeviceUtils::Compare compare) {
        __DBG_STUB_INVOKE__
        return compare ? compare("aaa", "a") : false;
    });

    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isSubpathOfDlnfs(""));

    EXPECT_TRUE(DeviceUtils::isSubpathOfDlnfs("xxx"));
}

TEST_F(UT_DeviceUtils, IsMountPointOfDlnfs)
{
    stub.set_lamda(DeviceUtils::findDlnfsPath, [](const QString &, DeviceUtils::Compare compare) {
        __DBG_STUB_INVOKE__
        return compare ? compare("aaa", "aaa") : false;
    });

    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isMountPointOfDlnfs(""));

    EXPECT_TRUE(DeviceUtils::isMountPointOfDlnfs("xxx"));
}

TEST_F(UT_DeviceUtils, HasMatch)
{
    // invalid inputs
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::hasMatch("", ""));

    EXPECT_TRUE(DeviceUtils::hasMatch("abcd", "abcd"));
}

TEST_F(UT_DeviceUtils, FindDlnfsPath)
{
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::findDlnfsPath("", [](const QString &target, const QString &compare) {
        return target.startsWith(compare);
    }));
}
