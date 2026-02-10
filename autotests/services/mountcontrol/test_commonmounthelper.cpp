// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include "mounthelpers/commonmounthelper.h"
#include "service_mountcontrol_global.h"

#include <QDBusContext>
#include <QVariantMap>

#include <libmount/libmount.h>
#include <sys/mount.h>
#include <errno.h>
#include <cstring>

SERVICEMOUNTCONTROL_USE_NAMESPACE

class UT_CommonMountHelper : public testing::Test
{
protected:
    virtual void SetUp() override {
        helper = new CommonMountHelper(nullptr);
    }
    
    virtual void TearDown() override { 
        stub.clear(); 
        delete helper;
    }

private:
    stub_ext::StubExt stub;
    CommonMountHelper *helper = nullptr;
    
protected:
    CommonMountHelper *getHelper() { return helper; }
};

TEST_F(UT_CommonMountHelper, Mount_ReturnsNotImplemented)
{
    // Test mount function returns not implemented error
    QString path = "/test/path";
    QVariantMap opts;
    
    QVariantMap result = getHelper()->mount(path, opts);
    
    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(), 
              QString("function is not implement"));
}

TEST_F(UT_CommonMountHelper, Unmount_EmptyPath_ReturnsTrue)
{
    // Test unmount with empty path returns success
    QString path = "";
    QVariantMap opts;
    
    QVariantMap result = getHelper()->unmount(path, opts);
    
    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(), 
              QString("no need to unmount empty path"));
}

TEST_F(UT_CommonMountHelper, Unmount_ParseMtabFailed_ReturnsFalse)
{
    // Test unmount when mtab parsing fails
    QString path = "/test/mount/path";
    QVariantMap opts;
    
    // Mock mnt_table_parse_mtab to return error
    stub.set_lamda(mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int {
        __DBG_STUB_INVOKE__
        return -1; // Simulate parse error
    });
    
    // Mock memory allocation functions
    stub.set_lamda(mnt_new_table, []() -> libmnt_table * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_table *>(0x1); // Return non-null pointer
    });
    
    stub.set_lamda(mnt_new_iter, [](int) -> libmnt_iter * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_iter *>(0x1); // Return non-null pointer
    });
    
    stub.set_lamda(mnt_free_table, [](libmnt_table *) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(mnt_free_iter, [](libmnt_iter *) {
        __DBG_STUB_INVOKE__
    });
    
    QVariantMap result = getHelper()->unmount(path, opts);
    
    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(), 
              QString("cannot parse mtab"));
}

TEST_F(UT_CommonMountHelper, Unmount_SuccessfulUnmount_ReturnsTrue)
{
    // Test successful unmount scenario
    QString path = "/test/mount/path/";
    QVariantMap opts;
    
    // Mock successful mtab parsing
    stub.set_lamda(mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int {
        __DBG_STUB_INVOKE__
        return 0; // Success
    });
    
    // Mock memory allocation functions
    stub.set_lamda(mnt_new_table, []() -> libmnt_table * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_table *>(0x1);
    });
    
    stub.set_lamda(mnt_new_iter, [](int) -> libmnt_iter * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_iter *>(0x1);
    });
    
    // Mock filesystem iteration - simulate finding matching mount
    int callCount = 0;
    stub.set_lamda(mnt_table_next_fs, [&callCount](libmnt_table *, libmnt_iter *, libmnt_fs **fs) -> int {
        __DBG_STUB_INVOKE__
        if (callCount == 0) {
            *fs = reinterpret_cast<libmnt_fs *>(0x1); // Valid fs pointer
            callCount++;
            return 0; // Found filesystem
        }
        return 1; // No more filesystems
    });
    
    // Mock filesystem info functions
    stub.set_lamda(mnt_fs_get_source, [](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        return "tmpfs"; // Not a block device
    });
    
    stub.set_lamda(mnt_fs_get_target, [](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/test/mount/path/subdir"; // Target that starts with our path
    });
    
    stub.set_lamda(mnt_free_table, [](libmnt_table *) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(mnt_free_iter, [](libmnt_iter *) {
        __DBG_STUB_INVOKE__
    });
    
    // Mock successful umount
    stub.set_lamda(umount, [](const char *) -> int {
        __DBG_STUB_INVOKE__
        return 0; // Success
    });
    
    QVariantMap result = getHelper()->unmount(path, opts);
    
    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
}

TEST_F(UT_CommonMountHelper, Unmount_UmountFailed_ReturnsFalse)
{
    // Test umount system call failure
    QString path = "/test/mount/path/";
    QVariantMap opts;
    
    // Mock successful mtab parsing
    stub.set_lamda(mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    
    stub.set_lamda(mnt_new_table, []() -> libmnt_table * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_table *>(0x1);
    });
    
    stub.set_lamda(mnt_new_iter, [](int) -> libmnt_iter * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_iter *>(0x1);
    });
    
    // Mock filesystem iteration
    int callCount = 0;
    stub.set_lamda(mnt_table_next_fs, [&callCount](libmnt_table *, libmnt_iter *, libmnt_fs **fs) -> int {
        __DBG_STUB_INVOKE__
        if (callCount == 0) {
            *fs = reinterpret_cast<libmnt_fs *>(0x1);
            callCount++;
            return 0;
        }
        return 1;
    });
    
    stub.set_lamda(mnt_fs_get_source, [](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        return "tmpfs";
    });
    
    stub.set_lamda(mnt_fs_get_target, [](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/test/mount/path/subdir";
    });
    
    stub.set_lamda(mnt_free_table, [](libmnt_table *) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(mnt_free_iter, [](libmnt_iter *) {
        __DBG_STUB_INVOKE__
    });
    
    // Mock failed umount
    stub.set_lamda(umount, [](const char *) -> int {
        __DBG_STUB_INVOKE__
        errno = EBUSY; // Device busy
        return -1; // Failure
    });
    
    QVariantMap result = getHelper()->unmount(path, opts);
    
    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), EBUSY);
    EXPECT_TRUE(result.value(MountReturnField::kErrorMessage).toString().contains("Device or resource busy"));
}

TEST_F(UT_CommonMountHelper, Unmount_SkipBlockDevices_OnlyUnmountsNonBlockDevices)
{
    // Test that block devices are skipped during unmount
    QString path = "/test/mount/path/";
    QVariantMap opts;
    
    // Mock successful mtab parsing
    stub.set_lamda(mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    
    stub.set_lamda(mnt_new_table, []() -> libmnt_table * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_table *>(0x1);
    });
    
    stub.set_lamda(mnt_new_iter, [](int) -> libmnt_iter * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<libmnt_iter *>(0x1);
    });
    
    // Mock filesystem iteration - simulate finding both block and non-block devices
    int callCount = 0;
    stub.set_lamda(mnt_table_next_fs, [&callCount](libmnt_table *, libmnt_iter *, libmnt_fs **fs) -> int {
        __DBG_STUB_INVOKE__
        if (callCount < 2) {
            *fs = reinterpret_cast<libmnt_fs *>(0x1);
            callCount++;
            return 0;
        }
        return 1;
    });
    
    int sourceCallCount = 0;
    stub.set_lamda(mnt_fs_get_source, [&sourceCallCount](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        sourceCallCount++;
        if (sourceCallCount == 1)
            return "/dev/sda1"; // Block device - should be skipped
        else
            return "tmpfs"; // Non-block device
    });
    
    int targetCallCount = 0;
    stub.set_lamda(mnt_fs_get_target, [&targetCallCount](libmnt_fs *) -> const char * {
        __DBG_STUB_INVOKE__
        targetCallCount++;
        return "/test/mount/path/subdir";
    });
    
    stub.set_lamda(mnt_free_table, [](libmnt_table *) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(mnt_free_iter, [](libmnt_iter *) {
        __DBG_STUB_INVOKE__
    });
    
    // Mock successful umount - should only be called once (for non-block device)
    int umountCallCount = 0;
    stub.set_lamda(umount, [&umountCallCount](const char *) -> int {
        __DBG_STUB_INVOKE__
        umountCallCount++;
        return 0;
    });
    
    QVariantMap result = getHelper()->unmount(path, opts);
    
    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(umountCallCount, 1); // Should only unmount the non-block device
}