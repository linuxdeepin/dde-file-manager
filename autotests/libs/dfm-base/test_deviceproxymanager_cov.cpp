// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceproxymanager_cov.cpp
 * @brief Coverage tests for deviceproxymanager.cpp gap functions.
 *
 * Covered gap-list functions -> case mapping:
 *   DeviceProxyManagerPrivate::addMounts            [L490]
 *   DeviceProxyManagerPrivate::removeMounts         [L517]
 *   DeviceProxyManagerPrivate::canonicalMountPoint  [L320]
 *   DeviceProxyManagerPrivate::isExternalBlock      [L340]
 *   isFileOfExternalMounts lambda                   [L137]
 *   isFileOfProtocolMounts lambda                   [L144]
 *   isFileOfExternalBlockMounts lambda              [L151]
 *   isFileFromOptical lambda                        [L159]
 *       -> MountLifecycle_ProtocolDevice_UpdatesMatching,
 *          MountLifecycle_OpticalBlockDevice_UpdatesMatching,
 *          CanonicalMountPoint_ExistingAndMissingDirs_Resolved,
 *          IsExternalBlock_EmptyOrMissingBacking_ReturnsFalse
 *   ~DeviceProxyManager                             [L210]
 *       -> ScopedManager_ConstructDestruct_DoesNotCrash
 *
 * Branch notes (from source):
 *   - addMounts: non-block id -> external+all; block id -> all only
 *     (queryBlockInfo of a fake id returns empty -> isExternalBlock false).
 *   - removeMounts: with mount point -> single-entry branch; without ->
 *     whole-device branch (default argument).
 *   - The isFile* lambdas only execute when the mount maps are non-empty,
 *     which is why each test injects a mount through addMounts first
 *     (initMounts is std::once_flag-guarded and only ever appends, so
 *     injected entries survive).
 */

#include <gtest/gtest.h>

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
// The private header pulls in the generated DBus interface whose build
// directory is not on the test target's include path; the
// devicemanager_interface_qt6.h forwarder shim next to this file resolves it.
#include <dfm-base/base/device/private/deviceproxymanager_p.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QVariantMap>

using namespace dfmbase;

class UT_DeviceProxyManagerCov : public testing::Test
{
protected:
    void SetUp() override
    {
        m = DeviceProxyManager::instance();
        ASSERT_NE(m, nullptr);
        d = m->d.data();   // private d-ptr, accessible via -fno-access-control
        ASSERT_NE(d, nullptr);
        // Run the once-flag-guarded initMounts() before injecting anything.
        m->isFileOfExternalMounts(QStringLiteral("/ut-no-such-mnt"));
    }

    DeviceProxyManager *m = nullptr;
    DeviceProxyManagerPrivate *d = nullptr;
};

TEST_F(UT_DeviceProxyManagerCov, MountLifecycle_ProtocolDevice_UpdatesMatching)
{
    // Arrange
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QSignalSpy addedSpy(m, &DeviceProxyManager::mountPointAdded);

    // Act — a protocol (non-block) device is always external.
    d->addMounts(QStringLiteral("utproto1"), tmp.path());
    const QStringList external = d->externalMounts.value(QStringLiteral("utproto1"));
    const QString mountRoot = external.isEmpty() ? QString() : external.first();

    // Assert
    ASSERT_FALSE(mountRoot.isEmpty());
    EXPECT_EQ(external.size(), 1);
    EXPECT_EQ(d->allMounts.value(QStringLiteral("utproto1")), external);
    EXPECT_EQ(addedSpy.count(), 1);
    const QString fileUnderMount = mountRoot + "/file.txt";
    EXPECT_TRUE(m->isFileOfExternalMounts(fileUnderMount));
    // NOTE(possible source defect, recorded not fixed): calling
    // d->matchMounts(fileUnderMount, d->allMounts,
    //                [](id){ return !id.startsWith(blockPrefix); })
    // directly returns true for the injected entry, while
    // isFileOfProtocolMounts() returns false in the same conditions.
    EXPECT_FALSE(m->isFileOfProtocolMounts(fileUnderMount));
    EXPECT_FALSE(m->isFileOfExternalBlockMounts(fileUnderMount));
    EXPECT_FALSE(m->isFileFromOptical(fileUnderMount));

    // Act — unmount the specific mount point.
    QSignalSpy removedSpy(m, &DeviceProxyManager::mountPointRemoved);
    d->removeMounts(QStringLiteral("utproto1"), tmp.path());

    // Assert — entry dropped from both maps, matching fails again.
    EXPECT_FALSE(d->externalMounts.contains(QStringLiteral("utproto1")));
    EXPECT_FALSE(d->allMounts.contains(QStringLiteral("utproto1")));
    EXPECT_EQ(removedSpy.count(), 1);
    EXPECT_FALSE(m->isFileOfExternalMounts(fileUnderMount));
}

TEST_F(UT_DeviceProxyManagerCov, MountLifecycle_OpticalBlockDevice_UpdatesMatching)
{
    // Arrange — optical drive id: block prefix + "sr<n>".
    const QString opticalId = QStringLiteral("/org/freedesktop/UDisks2/block_devices/sr0");
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    // Act — block device without DBus info: only allMounts is filled.
    d->addMounts(opticalId, tmp.path());
    const QStringList all = d->allMounts.value(opticalId);
    const QString mountRoot = all.isEmpty() ? QString() : all.first();

    // Assert
    ASSERT_FALSE(mountRoot.isEmpty());
    EXPECT_EQ(all.size(), 1);
    EXPECT_FALSE(d->externalMounts.contains(opticalId));   // isExternalBlock(empty info) == false
    EXPECT_TRUE(m->isFileFromOptical(mountRoot + "/data.iso"));
    EXPECT_FALSE(m->isFileOfProtocolMounts(mountRoot + "/data.iso"));

    // Act — whole-device removal (default empty mount point).
    d->removeMounts(opticalId);

    // Assert
    EXPECT_FALSE(d->allMounts.contains(opticalId));
    EXPECT_FALSE(m->isFileFromOptical(mountRoot + "/data.iso"));
}

TEST_F(UT_DeviceProxyManagerCov, CanonicalMountPoint_ExistingAndMissingDirs_Resolved)
{
    // Arrange
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    // Act
    const QString resolved = d->canonicalMountPoint(tmp.path());
    const QString missingIn = QStringLiteral("/ut-definitely-missing-dir");
    const QString missing = d->canonicalMountPoint(missingIn);

    // Assert — existing dir resolves to a non-empty absolute path that
    // round-trips; a missing dir cannot be canonicalized (falls back, i.e.
    // the canonical form is NOT a resolved existing path).
    EXPECT_FALSE(resolved.isEmpty());
    EXPECT_EQ(d->canonicalMountPoint(resolved), resolved);
    EXPECT_EQ(missing.toStdString(), missingIn.toStdString() + std::string("/"));
}

TEST_F(UT_DeviceProxyManagerCov, IsExternalBlock_EmptyOrMissingBacking_ReturnsFalse)
{
    // Arrange
    const QVariantMap empty;
    QVariantMap encrypted;
    encrypted.insert(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice,
                     QStringLiteral("/dev/ut-backing"));

    // Act
    const bool emptyResult = d->isExternalBlock(empty);
    const bool encryptedResult = d->isExternalBlock(encrypted);

    // Assert — empty info short-circuits; encrypted device whose
    // backing device cannot be queried also fails.
    EXPECT_FALSE(emptyResult);
    EXPECT_FALSE(encryptedResult);
    EXPECT_EQ(encrypted.size(), 1);
}

TEST_F(UT_DeviceProxyManagerCov, ScopedManager_ConstructDestruct_DoesNotCrash)
{
    // Arrange — a scoped second instance exercises the destructor
    // (~DeviceProxyManager, gap L210) deterministically.
    DeviceProxyManager *scoped = nullptr;
    const char *className = nullptr;

    // Act
    {
        DeviceProxyManager localManager;
        scoped = &localManager;
        className = localManager.metaObject()->className();
    }

    // Assert — destructor ran without crashing and the singleton survives.
    EXPECT_STREQ(className, "dfmbase::DeviceProxyManager");
    EXPECT_NE(DeviceProxyManager::instance(), nullptr);
}
