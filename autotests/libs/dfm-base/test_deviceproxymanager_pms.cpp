// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceproxymanager_pms.cpp
 * @brief PMS sev-2 regression tests for deviceproxymanager.cpp.
 *
 * Bug -> case mapping (fix commits verified via `git show`):
 *   - PMS:134883 (54dba828) mount point tail-slash normalization:
 *     matchMounts()/isMptOfDevice() normalize the queried path with a
 *     trailing '/' and store canonical mount points, so files under a
 *     mount root are matched and the mount root itself matches with or
 *     without the trailing slash.
 *   - PMS:347531 (371b050b) DeviceProxyManagerPrivate::isExternalBlock:
 *     encrypted partitions must be classified through their crypto
 *     backing device (removable backing => external).
 *   - PMS:338063 (af2f096b) DeviceProxyManagerPrivate::canonicalMountPoint:
 *     pure string normalization via g_canonicalize_filename, no file IO,
 *     so non-existent paths are still canonicalized and always end
 *     with '/'.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
// The private header pulls in the generated DBus interface whose build
// directory is not on the test target's include path; the
// devicemanager_interface_qt6.h forwarder shim next to this file resolves it.
#include <dfm-base/base/device/private/deviceproxymanager_p.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include "stubext.h"

#include <QTemporaryDir>
#include <QVariantMap>

using namespace dfmbase;
using namespace GlobalServerDefines;

class UT_DeviceProxyManagerPms : public testing::Test
{
protected:
    void SetUp() override
    {
        m = DeviceProxyManager::instance();
        ASSERT_NE(m, nullptr);
        d = m->d.data();   // private d-ptr, accessible via -fno-access-control
        ASSERT_NE(d, nullptr);
        // Run the once-flag-guarded initMounts() before injecting anything.
        m->isFileOfExternalMounts(QStringLiteral("/ut-pms-no-such-mnt"));
    }

    void TearDown() override
    {
        stub.clear();
    }

    DeviceProxyManager *m = nullptr;
    DeviceProxyManagerPrivate *d = nullptr;
    stub_ext::StubExt stub;
};

// PMS:134883 挂载点路径尾斜杠匹配错误：新挂载的U盘/手机目录下的文件识别不到外部设备
// 修复后挂载点统一规范化为以 '/' 结尾，matchMounts 用 startsWith 匹配挂载点下的文件
TEST_F(UT_DeviceProxyManagerPms, BUG134883_FileUnderProtocolMountDetected)
{
    // Arrange — a protocol (non-block) device id is always classified external.
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    d->addMounts(QStringLiteral("utpms-134883"), tmp.path());
    const QStringList external = d->externalMounts.value(QStringLiteral("utpms-134883"));
    ASSERT_EQ(external.size(), 1);
    // Canonical mount point always ends with '/' (tail-slash normalization).
    EXPECT_TRUE(external.first().endsWith('/'));

    // Act / Assert — files below the mount root match, with and without the
    // trailing separator on the queried path.
    EXPECT_TRUE(m->isFileOfExternalMounts(tmp.path() + "/DCIM/photo.jpg"));
    EXPECT_TRUE(m->isFileOfExternalMounts(tmp.path() + "/"));
    EXPECT_TRUE(m->isFileOfExternalMounts(tmp.path()));
    // A different directory must not match.
    QTemporaryDir other;
    ASSERT_TRUE(other.isValid());
    EXPECT_FALSE(m->isFileOfExternalMounts(other.path() + "/other.txt"));

    d->removeMounts(QStringLiteral("utpms-134883"));
    EXPECT_FALSE(m->isFileOfExternalMounts(tmp.path() + "/DCIM/photo.jpg"));
}

// PMS:134883 isMptOfDevice 需要在查询路径缺少尾斜杠时仍能命中挂载根
TEST_F(UT_DeviceProxyManagerPms, BUG134883_IsMptOfDeviceMatchesMountRoot)
{
    // Arrange
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    d->addMounts(QStringLiteral("utpms-134883b"), tmp.path());

    // Act / Assert — both "path" and "path/" resolve to the same mount entry.
    QString id;
    EXPECT_TRUE(m->isMptOfDevice(tmp.path() + "/", id));
    EXPECT_EQ(id, QStringLiteral("utpms-134883b"));

    EXPECT_TRUE(m->isMptOfDevice(tmp.path(), id));
    EXPECT_EQ(id, QStringLiteral("utpms-134883b"));

    // A path below the mount root is not itself a mount point.
    EXPECT_FALSE(m->isMptOfDevice(tmp.path() + "/sub", id));

    d->removeMounts(QStringLiteral("utpms-134883b"));
}

// PMS:347531 加密分区被误判为内置磁盘：isExternalBlock 必须通过加密后备设备判断
TEST_F(UT_DeviceProxyManagerPms, BUG347531_EncryptedBlockResolvedViaBackingDevice)
{
    // Arrange — stub queryBlockInfo: the backing device of the encrypted
    // partition is a removable USB disk.
    QVariantMap backing;
    backing.insert(DeviceProperty::kRemovable, true);
    QString queriedId;
    stub.set_lamda(static_cast<QVariantMap (DeviceProxyManager::*)(const QString &, bool)>(&DeviceProxyManager::queryBlockInfo),
                   [&backing, &queriedId](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
                       queriedId = id;
                       return backing;
                   });

    QVariantMap info;
    info.insert(DeviceProperty::kCryptoBackingDevice, QStringLiteral("/dev/ut-crypt-347531"));
    info.insert(DeviceProperty::kRemovable, false);   // the partition itself reports non-removable

    // Act / Assert — the encrypted partition is classified through the
    // removable backing device and therefore is external.
    EXPECT_TRUE(d->isExternalBlock(info));
    EXPECT_EQ(queriedId, QStringLiteral("/dev/ut-crypt-347531"));

    // The same classification for a plain removable device (no backing device).
    QVariantMap plainRemovable;
    plainRemovable.insert(DeviceProperty::kRemovable, true);
    EXPECT_TRUE(d->isExternalBlock(plainRemovable));

    // A non-removable plain device is never external.
    QVariantMap plainFixed;
    plainFixed.insert(DeviceProperty::kRemovable, false);
    EXPECT_FALSE(d->isExternalBlock(plainFixed));
}

// PMS:347531 加密后备设备信息查询失败时不得崩溃，且不误报为外部设备
TEST_F(UT_DeviceProxyManagerPms, BUG347531_MissingBackingDeviceInfoReturnsFalse)
{
    // Arrange — queryBlockInfo returns an empty map (device gone).
    stub.set_lamda(static_cast<QVariantMap (DeviceProxyManager::*)(const QString &, bool)>(&DeviceProxyManager::queryBlockInfo),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       return {};
                   });

    QVariantMap info;
    info.insert(DeviceProperty::kCryptoBackingDevice, QStringLiteral("/dev/ut-gone-347531"));

    // Act / Assert — no crash and a conservative "not external" answer.
    EXPECT_FALSE(d->isExternalBlock(info));

    // Empty device info is rejected as well.
    EXPECT_FALSE(d->isExternalBlock(QVariantMap()));
}

// PMS:338063 canonicalMountPoint 改为纯字符串规范化（无文件 IO），不存在路径也能规范化
TEST_F(UT_DeviceProxyManagerPms, BUG338063_CanonicalMountPointPureStringNormalization)
{
    // Arrange / Act / Assert
    // Path traversal segments are resolved without touching the filesystem.
    EXPECT_EQ(d->canonicalMountPoint(QStringLiteral("/a/b/../c")), QStringLiteral("/a/c/"));
    // A non-existent path is still canonicalized and terminated with '/'.
    EXPECT_EQ(d->canonicalMountPoint(QStringLiteral("/ut/pms/338063/not-exist")),
              QStringLiteral("/ut/pms/338063/not-exist/"));
    // A directory path is not duplicated with a trailing slash.
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    EXPECT_EQ(d->canonicalMountPoint(tmp.path()), tmp.path() + QStringLiteral("/"));
    // Empty input stays empty.
    EXPECT_TRUE(d->canonicalMountPoint(QString()).isEmpty());
}
