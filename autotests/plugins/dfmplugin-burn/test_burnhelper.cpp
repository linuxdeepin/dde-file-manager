// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <DDialog>

#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPBURN_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_BurnHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });
    }
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnHelper, IsBurnEnabled)
{
    bool ret { false };
    stub.set_lamda(&DConfigManager::value, [&] { __DBG_STUB_INVOKE__ return ret; });
    EXPECT_FALSE(BurnHelper::isBurnEnabled());

    ret = true;
    EXPECT_TRUE(BurnHelper::isBurnEnabled());
}

TEST_F(UT_BurnHelper, IsBurnEnabled_InvalidValue)
{
    stub.set_lamda(&DConfigManager::value, [] {
        __DBG_STUB_INVOKE__
        return QVariant();   // Invalid value
    });
    EXPECT_TRUE(BurnHelper::isBurnEnabled());   // Should return default true
}

TEST_F(UT_BurnHelper, showOpticalBlankConfirmationDialog)
{
    int expectedResult = 1;
    stub.set_lamda(VADDR(DDialog, exec), [&expectedResult] {
        __DBG_STUB_INVOKE__
        return expectedResult;
    });

    int result = BurnHelper::showOpticalBlankConfirmationDialog();
    EXPECT_EQ(result, expectedResult);
}

TEST_F(UT_BurnHelper, showOpticalImageOpSelectionDialog)
{
    int expectedResult = 2;
    stub.set_lamda(VADDR(DDialog, exec), [&expectedResult] {
        __DBG_STUB_INVOKE__
        return expectedResult;
    });

    int result = BurnHelper::showOpticalImageOpSelectionDialog();
    EXPECT_EQ(result, expectedResult);
}

TEST_F(UT_BurnHelper, localStagingFile_WithDevice)
{
    QString testDev = "/dev/sr0";

    stub.set_lamda(&QStandardPaths::writableLocation, [] {
        __DBG_STUB_INVOKE__
        return QString("/tmp/cache");
    });

    stub.set_lamda(&QApplication::organizationName, [] {
        __DBG_STUB_INVOKE__
        return QString("deepin");
    });

    QUrl result = BurnHelper::localStagingFile(testDev);
    EXPECT_TRUE(result.isLocalFile());
    EXPECT_TRUE(result.toLocalFile().contains("_dev_sr0"));
}

TEST_F(UT_BurnHelper, localStagingFile_WithUrl)
{
    QUrl destUrl;
    destUrl.setScheme("burn");
    destUrl.setPath("/dev/sr0/disc_files/test.txt");

    stub.set_lamda(ADDR(BurnHelper, burnDestDevice), [] {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(ADDR(BurnHelper, burnFilePath), [] {
        __DBG_STUB_INVOKE__
        return QString("/test.txt");
    });

    stub.set_lamda(&QStandardPaths::writableLocation, [] {
        __DBG_STUB_INVOKE__
        return QString("/tmp/cache");
    });

    stub.set_lamda(&QApplication::organizationName, [] {
        __DBG_STUB_INVOKE__
        return QString("deepin");
    });

    QUrl result = BurnHelper::localStagingFile(destUrl);
    EXPECT_TRUE(result.isLocalFile());
}

TEST_F(UT_BurnHelper, localStagingFile_WithUrl_EmptyDevice)
{
    QUrl destUrl;
    destUrl.setScheme("burn");
    destUrl.setPath("/invalid/path");

    stub.set_lamda(ADDR(BurnHelper, burnDestDevice), [] {
        __DBG_STUB_INVOKE__
        return QString();   // Empty device
    });

    QUrl result = BurnHelper::localStagingFile(destUrl);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_BurnHelper, fromBurnFile)
{
    QString testDev = "/dev/sr0";

    QUrl result = BurnHelper::fromBurnFile(testDev);
    EXPECT_EQ(result.scheme(), "burn");
    EXPECT_TRUE(result.path().contains("staging_files"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_DuplicateFile)
{
    QStringList messages;
    // The method expects both conditions in the same message string
    messages << "While grafting '/path/to/file.txt' file object exists and may not be overwritten";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("duplicate file"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_InsufficientSpace)
{
    QStringList messages;
    messages << "Image size 1024 exceeds free space on media 512";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("Insufficient disc space"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_LostConnection)
{
    QStringList messages;
    messages << "Lost connection to drive";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("Lost connection to drive"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_ServoFailure)
{
    QStringList messages;
    messages << "servo failure";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("not ready"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_DeviceBusy)
{
    QStringList messages;
    messages << "Device or resource busy";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("busy"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_InvalidVolumeName)
{
    QStringList messages;
    messages << "-volid: Text too long";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("Invalid volume name"));
}

TEST_F(UT_BurnHelper, parseXorrisoErrorMessage_UnknownError)
{
    QStringList messages;
    messages << "Some unknown error message";

    QString result = BurnHelper::parseXorrisoErrorMessage(messages);
    EXPECT_TRUE(result.contains("Unknown error"));
}

TEST_F(UT_BurnHelper, burnDestDevice_ValidBurnUrl)
{
    QUrl url;
    url.setScheme("burn");
    url.setPath("/dev/sr0/disc_files/test.txt");

    QString result = BurnHelper::burnDestDevice(url);
    EXPECT_EQ(result, "/dev/sr0");
}

TEST_F(UT_BurnHelper, burnDestDevice_InvalidScheme)
{
    QUrl url;
    url.setScheme("file");
    url.setPath("/dev/sr0/disc_files/test.txt");

    QString result = BurnHelper::burnDestDevice(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_BurnHelper, burnDestDevice_InvalidPath)
{
    QUrl url;
    url.setScheme("burn");
    url.setPath("/invalid/path");

    QString result = BurnHelper::burnDestDevice(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_BurnHelper, burnFilePath_ValidBurnUrl)
{
    QUrl url;
    url.setScheme("burn");
    url.setPath("/dev/sr0/disc_files/test.txt");

    QString result = BurnHelper::burnFilePath(url);
    EXPECT_EQ(result, "/test.txt");
}

TEST_F(UT_BurnHelper, burnFilePath_InvalidScheme)
{
    QUrl url;
    url.setScheme("file");
    url.setPath("/dev/sr0/disc_files/test.txt");

    QString result = BurnHelper::burnFilePath(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_BurnHelper, discDataGroup)
{
    QStringList mockIds = { "/org/freedesktop/UDisks2/block_devices/sr0",
                            "/org/freedesktop/UDisks2/block_devices/sda1" };

    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [&mockIds] {
        __DBG_STUB_INVOKE__
        return mockIds;
    });

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [](DeviceProxyManager *, const QString &id, bool needDBusCall) {
        __DBG_STUB_INVOKE__
        Q_UNUSED(needDBusCall)
        QVariantMap data;
        if (id.contains("sr0")) {
            data[DeviceProperty::kOptical] = true;
            data[DeviceProperty::kOpticalDrive] = true;
        } else {
            data[DeviceProperty::kOptical] = false;
            data[DeviceProperty::kOpticalDrive] = false;
        }
        return data;
    });

    QList<QVariantMap> result = BurnHelper::discDataGroup();
    EXPECT_EQ(result.size(), 1);   // Only sr0 should be included
}

TEST_F(UT_BurnHelper, updateBurningStateToPersistence)
{
    bool setCalled = false;
    bool syncCalled = false;

    using SetValueFunc = void (Settings::*)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValueFunc>(&Settings::setValue), [&setCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        setCalled = true;
    });

    stub.set_lamda(ADDR(Settings, sync), [&syncCalled](Settings *) {
        __DBG_STUB_INVOKE__
        syncCalled = true;
        return true;
    });

    // Create a mock Settings object for testing
    static Settings *mockSettings = new Settings("test", Settings::kAppConfig);
    stub.set_lamda(ADDR(Application, dataPersistence), []() -> Settings * {
        __DBG_STUB_INVOKE__
        static Settings *instance = new Settings("test", Settings::kAppConfig);
        return instance;   // Return valid Settings object
    });

    BurnHelper::updateBurningStateToPersistence("test_id", "/dev/sr0", true);

    EXPECT_TRUE(setCalled);
    EXPECT_TRUE(syncCalled);
}

TEST_F(UT_BurnHelper, mapStagingFilesPath_EmptyList)
{
    QList<QUrl> emptyList;

    // Should not crash with empty lists
    BurnHelper::mapStagingFilesPath(emptyList, emptyList);
}

TEST_F(UT_BurnHelper, mapStagingFilesPath_SizeMismatch)
{
    QList<QUrl> srcList = { QUrl::fromLocalFile("/src/file1.txt") };
    QList<QUrl> targetList = { QUrl::fromLocalFile("/target/file1.txt"),
                               QUrl::fromLocalFile("/target/file2.txt") };

    // Should handle size mismatch gracefully
    BurnHelper::mapStagingFilesPath(srcList, targetList);
}

TEST_F(UT_BurnHelper, burnIsOnLocalStaging_ValidPath)
{
    QUrl url = QUrl::fromLocalFile("/home/user/.cache/deepin/discburn/_dev_sr0/test.txt");

    bool result = BurnHelper::burnIsOnLocalStaging(url);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnHelper, burnIsOnLocalStaging_InvalidPath)
{
    QUrl url = QUrl::fromLocalFile("/home/user/Documents/test.txt");

    bool result = BurnHelper::burnIsOnLocalStaging(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnHelper, localFileInfoList_ValidDirectory)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Create test files
    QFile file1(tempDir.path() + "/file1.txt");
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.close();

    QDir subDir(tempDir.path() + "/subdir");
    ASSERT_TRUE(subDir.mkpath("."));

    QFileInfoList result = BurnHelper::localFileInfoList(tempDir.path());
    EXPECT_EQ(result.size(), 2);   // file1.txt and subdir
}

TEST_F(UT_BurnHelper, localFileInfoList_NonExistentDirectory)
{
    QFileInfoList result = BurnHelper::localFileInfoList("/non/existent/path");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_BurnHelper, localFileInfoListRecursive_ValidDirectory)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Create test files
    QFile file1(tempDir.path() + "/file1.txt");
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.close();

    QDir subDir(tempDir.path() + "/subdir");
    ASSERT_TRUE(subDir.mkpath("."));

    QFile file2(tempDir.path() + "/subdir/file2.txt");
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();

    QFileInfoList result = BurnHelper::localFileInfoListRecursive(tempDir.path());
    EXPECT_EQ(result.size(), 2);   // file1.txt and file2.txt (only files, not dirs)
}

TEST_F(UT_BurnHelper, localFileInfoListRecursive_NonExistentDirectory)
{
    QFileInfoList result = BurnHelper::localFileInfoListRecursive("/non/existent/path");
    EXPECT_TRUE(result.isEmpty());
}
