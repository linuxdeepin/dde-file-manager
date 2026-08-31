// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_freestanding.cpp
 * @brief Unit tests for FreeStanding Low-priority methods
 */

#include <gtest/gtest.h>

class FreeStandingLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FreeStandingLowTest, DSMRegister)
{
    EXPECT_NO_FATAL_FAILURE({ DSMRegister(nullptr, nullptr); });
}

TEST_F(FreeStandingLowTest, DSMUnRegister)
{
    EXPECT_NO_FATAL_FAILURE({ DSMUnRegister(nullptr, nullptr); });
}

TEST_F(FreeStandingLowTest, ScopeGuard)
{
    EXPECT_NO_FATAL_FAILURE({ ScopeGuard(); });
}

TEST_F(FreeStandingLowTest, abrecovery_helper)
{
    abrecovery_helper *obj = new abrecovery_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->disableRecovery(); });
    delete obj;
}

TEST_F(FreeStandingLowTest, blockdev_helper)
{
    blockdev_helper *obj = new blockdev_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->devDeviceSize(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->devBlockSize(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->createDevPtr(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->devCryptVersion(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->createDevPtr2(_arg0); });
    delete obj;
}

TEST_F(FreeStandingLowTest, common_helper)
{
    common_helper *obj = new common_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->encryptCipher(); });
    EXPECT_NO_FATAL_FAILURE({ obj->genRecoveryKey(); });
    EXPECT_NO_FATAL_FAILURE({ obj->genRandomString(0); });
    EXPECT_NO_FATAL_FAILURE({ obj->createDFMDesktopEntry(); });
    EXPECT_NO_FATAL_FAILURE(obj->createRebootFlagFile(_arg0));
    delete obj;
}

TEST_F(FreeStandingLowTest, crypttab_helper)
{
    crypttab_helper *obj = new crypttab_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->cryptItems(_arg0); });
    EXPECT_NO_FATAL_FAILURE(obj->updateInitramfs());
    EXPECT_NO_FATAL_FAILURE({ obj->addCryptOption(_arg0, _arg1); });
    EXPECT_NO_FATAL_FAILURE({ obj->updateCryptTab(); });
    EXPECT_NO_FATAL_FAILURE({ obj->insertCryptItem(_arg0); });
    delete obj;
}

TEST_F(FreeStandingLowTest, dfm_setup_accesscontrol_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_accesscontrol_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_accesscontrol_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_accesscontrol_dependencies());
}

TEST_F(FreeStandingLowTest, dfm_setup_diskencrypt_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_diskencrypt_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_diskencrypt_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_diskencrypt_dependencies());
}

TEST_F(FreeStandingLowTest, dfm_setup_mountcontrol_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_mountcontrol_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_mountcontrol_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_mountcontrol_dependencies());
}

TEST_F(FreeStandingLowTest, dfm_setup_sharecontrol_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_sharecontrol_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_sharecontrol_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_sharecontrol_dependencies());
}

TEST_F(FreeStandingLowTest, dfm_setup_textindex_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_textindex_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_tpmcontrol_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_tpmcontrol_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_tpmcontrol_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_tpmcontrol_dependencies());
}

TEST_F(FreeStandingLowTest, dfm_setup_usbrepair_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_usbrepair_dbus_interfaces());
}

TEST_F(FreeStandingLowTest, dfm_setup_usbrepair_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_usbrepair_dependencies());
}

TEST_F(FreeStandingLowTest, filesystem_helper)
{
    filesystem_helper *obj = new filesystem_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->remountBoot());
    EXPECT_NO_FATAL_FAILURE({ obj->moveFsForward(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->shrinkFileSystem_ext(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->expandFileSystem_ext(_arg0); });
    delete obj;
}

TEST_F(FreeStandingLowTest, fstab_helper)
{
    fstab_helper *obj = new fstab_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->setFstabPassno(_arg0, 0); });
    EXPECT_NO_FATAL_FAILURE({ obj->setFstabTimeout(_arg0, _arg1); });
    delete obj;
}

TEST_F(FreeStandingLowTest, inhibit_helper)
{
    EXPECT_NO_FATAL_FAILURE({ inhibit_helper(); });
}

TEST_F(FreeStandingLowTest, invokeProxyExtract)
{
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ invokeProxyExtract(nullptr, _arg1, {}); });
}

TEST_F(FreeStandingLowTest, job_file_helper)
{
    job_file_helper *obj = new job_file_helper();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->checkJobs());
    EXPECT_NO_FATAL_FAILURE({ obj->removeJobFile(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->hasJobFile(); });
    EXPECT_NO_FATAL_FAILURE({ obj->validJobTypes(); });
    EXPECT_NO_FATAL_FAILURE({ obj->createEncryptJobFile(_arg0); });
    delete obj;
}

TEST_F(FreeStandingLowTest, mergedSmbUrl)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ mergedSmbUrl(_arg0); });
}

TEST_F(FreeStandingLowTest, udev_helper)
{
    EXPECT_NO_FATAL_FAILURE({ udev_helper(); });
}

