// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-burn/dburn_global.h>

#include <QCoreApplication>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

DFMBASE_USE_NAMESPACE
DFM_BURN_USE_NS
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class TestOpticalHelper : public testing::Test
{
public:
    void SetUp() override
    {
        helper = OpticalHelper::instance();
        setupDefaultMocks();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    void setupDefaultMocks()
    {
        // Mock QStandardPaths
        stub.set_lamda(&QStandardPaths::writableLocation, [](QStandardPaths::StandardLocation type) {
            __DBG_STUB_INVOKE__
            if (type == QStandardPaths::GenericCacheLocation) {
                return QString("/tmp/cache");
            }
            return QString("/tmp");
        });

        // Mock QCoreApplication
        stub.set_lamda(&QCoreApplication::organizationName, []() {
            __DBG_STUB_INVOKE__
            return QString("deepin");
        });

        // Mock DeviceUtils
        stub.set_lamda(&DeviceUtils::getMountInfo, [this](const QString &devFile, bool) {
            __DBG_STUB_INVOKE__
            return mockMountPoint;
        });

        // Mock DeviceProxyManager
        stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [this]() {
            __DBG_STUB_INVOKE__
            return mockBlockIds;
        });

        stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [this]() {
            __DBG_STUB_INVOKE__
            QVariantMap map;
            map[DeviceProperty::kMountPoints] = mockMountPoint;
            return map;
        });

        // Mock FileInfo
        stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, NameInfoType type) {
            __DBG_STUB_INVOKE__
            return QString("test.txt");
        });

        // Mock QDir
        using ExistsFuncPtr = bool (QDir::*)() const;
        stub.set_lamda(static_cast<ExistsFuncPtr>(&QDir::exists), [this](QDir *) {
            __DBG_STUB_INVOKE__
            return mockDirExists;
        });

        using EntryInfoListFuncPtr = QFileInfoList (QDir::*)(QDir::Filters, QDir::SortFlags) const;
        stub.set_lamda(static_cast<EntryInfoListFuncPtr>(&QDir::entryInfoList), [this]() {
            __DBG_STUB_INVOKE__
            return mockFileInfoList;
        });

        stub.set_lamda(&QDir::mkpath, []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Mock DConfigManager
        stub.set_lamda(&DConfigManager::value, [this](DConfigManager *, const QString &key1, const QString &key2, const QVariant &) {
            __DBG_STUB_INVOKE__
            if (key2 == "burnEnable") {
                return QVariant(mockBurnEnabled);
            }
            return QVariant();
        });

        stub.set_lamda(&DConfigManager::instance, []() {
            __DBG_STUB_INVOKE__
            static DConfigManager manager;
            return &manager;
        });
    }

    OpticalHelper *helper = nullptr;
    stub_ext::StubExt stub;

    // Mock data
    QString mockMountPoint = "/media/disc";
    QStringList mockBlockIds = { "block_device_1", "block_device_2" };
    QSharedPointer<FileInfo> mockFileInfo;
    bool mockDirExists = true;
    QFileInfoList mockFileInfoList;
    bool mockBurnEnabled = true;
};

TEST_F(TestOpticalHelper, Instance_ReturnsSingleton)
{
    OpticalHelper *instance1 = OpticalHelper::instance();
    OpticalHelper *instance2 = OpticalHelper::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(TestOpticalHelper, Scheme_ReturnsCorrectScheme)
{
    QString scheme = OpticalHelper::scheme();
    EXPECT_EQ(scheme, Global::Scheme::kBurn);
}

TEST_F(TestOpticalHelper, Icon_ReturnsValidIcon)
{
    QIcon icon = OpticalHelper::icon();
    EXPECT_FALSE(icon.isNull());
}

TEST_F(TestOpticalHelper, IconString_ReturnsCorrectString)
{
    QString iconString = OpticalHelper::iconString();
    EXPECT_EQ(iconString, "media-optical-symbolic");
}

TEST_F(TestOpticalHelper, DiscRoot_ValidDevice_ReturnsCorrectUrl)
{
    QString device = "/dev/sr0";
    QUrl result = OpticalHelper::discRoot(device);

    EXPECT_EQ(result.scheme(), "burn");
    EXPECT_TRUE(result.path().contains("/dev/sr0/disc_files/"));
}

TEST_F(TestOpticalHelper, LocalStagingRoot_ReturnsCorrectPath)
{
    QUrl result = OpticalHelper::localStagingRoot();

    EXPECT_TRUE(result.isLocalFile());
    EXPECT_TRUE(result.path().contains("cache"));
    EXPECT_TRUE(result.path().contains("deepin"));
    EXPECT_TRUE(result.path().contains("discburn"));
}

TEST_F(TestOpticalHelper, LocalStagingFile_ValidDest_ReturnsCorrectPath)
{
    QUrl dest("burn:///dev/sr0/disc_files/test.txt");
    QUrl result = OpticalHelper::localStagingFile(dest);

    EXPECT_TRUE(result.isLocalFile());
    EXPECT_TRUE(result.path().contains("cache"));
    EXPECT_TRUE(result.path().contains("deepin"));
    EXPECT_TRUE(result.path().contains("discburn"));
}

TEST_F(TestOpticalHelper, LocalStagingFile_EmptyDevice_ReturnsEmptyUrl)
{
    QUrl dest("invalid://test");

    // Mock burnDestDevice to return empty string
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    QUrl result = OpticalHelper::localStagingFile(dest);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, LocalStagingFileByDevice_ValidDevice_ReturnsCorrectPath)
{
    QString device = "/dev/sr0";
    QUrl result = OpticalHelper::localStagingFile(device);

    EXPECT_TRUE(result.isLocalFile());
    EXPECT_TRUE(result.path().contains("cache"));
    EXPECT_TRUE(result.path().contains("deepin"));
    EXPECT_TRUE(result.path().contains("discburn"));
    EXPECT_TRUE(result.path().contains("_dev_sr0"));   // Device path with slashes replaced
}

TEST_F(TestOpticalHelper, LocalDiscFile_ValidDest_ReturnsCorrectPath)
{
    QUrl dest("burn:///dev/sr0/disc_files/test.txt");

    // Mock burnDestDevice
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    // Mock burnFilePath
    stub.set_lamda(&OpticalHelper::burnFilePath, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/test.txt");
    });

    QUrl result = OpticalHelper::localDiscFile(dest);

    EXPECT_TRUE(result.isLocalFile());
    EXPECT_EQ(result.path(), mockMountPoint + "/test.txt");
}

TEST_F(TestOpticalHelper, LocalDiscFile_EmptyDevice_ReturnsEmptyUrl)
{
    QUrl dest("burn:///dev/sr0/disc_files/test.txt");

    // Mock burnDestDevice to return empty
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    QUrl result = OpticalHelper::localDiscFile(dest);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, LocalDiscFile_EmptyMountPoint_ReturnsEmptyUrl)
{
    QUrl dest("burn:///dev/sr0/disc_files/test.txt");

    // Mock burnDestDevice
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    mockMountPoint = "";   // Empty mount point

    QUrl result = OpticalHelper::localDiscFile(dest);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, BurnDestDevice_ValidBurnUrl_ReturnsDevice)
{
    QUrl url("burn:///dev/sr0/disc_files/test.txt");
    QString result = OpticalHelper::burnDestDevice(url);

    EXPECT_EQ(result, "/dev/sr0");
}

TEST_F(TestOpticalHelper, BurnDestDevice_InvalidScheme_ReturnsEmpty)
{
    QUrl url("file:///tmp/test.txt");
    QString result = OpticalHelper::burnDestDevice(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, BurnDestDevice_InvalidPath_ReturnsEmpty)
{
    QUrl url("burn:///invalid/path");
    QString result = OpticalHelper::burnDestDevice(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, BurnFilePath_ValidBurnUrl_ReturnsFilePath)
{
    QUrl url("burn:///dev/sr0/disc_files/folder/test.txt");
    QString result = OpticalHelper::burnFilePath(url);

    EXPECT_EQ(result, "/folder/test.txt");
}

TEST_F(TestOpticalHelper, BurnFilePath_InvalidUrl_ReturnsEmpty)
{
    QUrl url("file:///tmp/test.txt");
    QString result = OpticalHelper::burnFilePath(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, BurnIsOnDisc_DiscFilesUrl_ReturnsTrue)
{
    QUrl url("burn:///dev/sr0/disc_files/test.txt");
    bool result = OpticalHelper::burnIsOnDisc(url);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, BurnIsOnDisc_StagingFilesUrl_ReturnsFalse)
{
    QUrl url("burn:///dev/sr0/staging_files/test.txt");
    bool result = OpticalHelper::burnIsOnDisc(url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, BurnIsOnDisc_InvalidUrl_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    bool result = OpticalHelper::burnIsOnDisc(url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, BurnIsOnStaging_StagingFilesUrl_ReturnsTrue)
{
    QUrl url("burn:///dev/sr0/staging_files/test.txt");
    bool result = OpticalHelper::burnIsOnStaging(url);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, BurnIsOnStaging_DiscFilesUrl_ReturnsFalse)
{
    QUrl url("burn:///dev/sr0/disc_files/test.txt");
    bool result = OpticalHelper::burnIsOnStaging(url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, BurnIsOnStaging_InvalidUrl_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    bool result = OpticalHelper::burnIsOnStaging(url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, TansToBurnFile_ValidCachePath_ReturnsBurnUrl)
{
    QUrl cacheUrl("/tmp/cache/deepin/discburn/_dev_sr0/test.txt");
    QUrl result = OpticalHelper::tansToBurnFile(cacheUrl);

    EXPECT_EQ(result.scheme(), "burn");
    EXPECT_TRUE(result.path().contains("/dev/sr0/staging_files/test.txt"));
}

TEST_F(TestOpticalHelper, TransDiscRootById_ValidId_ReturnsDiscRootUrl)
{
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QUrl result = OpticalHelper::transDiscRootById(id);

    EXPECT_EQ(result.scheme(), "burn");
    EXPECT_TRUE(result.path().contains("/dev/sr0/disc_files/"));
}

TEST_F(TestOpticalHelper, TransDiscRootById_InvalidId_ReturnsEmptyUrl)
{
    QString id = "/invalid/id/format";
    QUrl result = OpticalHelper::transDiscRootById(id);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, IsSupportedUDFVersion_SupportedVersion_ReturnsTrue)
{
    QString version = "1.02";
    bool result = OpticalHelper::isSupportedUDFVersion(version);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, IsSupportedUDFVersion_UnsupportedVersion_ReturnsFalse)
{
    QString version = "2.01";
    bool result = OpticalHelper::isSupportedUDFVersion(version);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsSupportedUDFMedium_SupportedMedium_ReturnsTrue)
{
    int dvdRType = static_cast<int>(MediaType::kDVD_R);
    bool result = OpticalHelper::isSupportedUDFMedium(dvdRType);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, IsSupportedUDFMedium_UnsupportedMedium_ReturnsFalse)
{
    int bluRayType = static_cast<int>(MediaType::kBD_ROM);
    bool result = OpticalHelper::isSupportedUDFMedium(bluRayType);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, CreateStagingFolder_ValidOpticalDevice_CreatesFolder)
{
    QString device = "/dev/sr0";

    bool mkpathCalled = false;
    stub.set_lamda(&QDir::mkpath, [&](QDir *, const QString &dirPath) {
        __DBG_STUB_INVOKE__
        mkpathCalled = true;
        return true;
    });

    // Mock QFileInfo to indicate directory doesn't exist
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    stub.set_lamda(exists, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    OpticalHelper::createStagingFolder(device);

    EXPECT_TRUE(mkpathCalled);
}

TEST_F(TestOpticalHelper, CreateStagingFolder_NonOpticalDevice_SkipsCreation)
{
    QString device = "/dev/sda1";   // Not an optical device

    bool mkpathCalled = false;
    stub.set_lamda(&QDir::mkpath, [&](QDir *, const QString &dirPath) {
        __DBG_STUB_INVOKE__
        mkpathCalled = true;
        return true;
    });

    OpticalHelper::createStagingFolder(device);

    EXPECT_FALSE(mkpathCalled);
}

TEST_F(TestOpticalHelper, CreateStagingFolder_ExistingFolder_SkipsCreation)
{
    QString device = "/dev/sr0";

    bool mkpathCalled = false;
    stub.set_lamda(&QDir::mkpath, [&](QDir *, const QString &dirPath) {
        __DBG_STUB_INVOKE__
        mkpathCalled = true;
        return true;
    });

    // Mock QFileInfo to indicate directory exists
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    stub.set_lamda(exists, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    OpticalHelper::createStagingFolder(device);

    EXPECT_FALSE(mkpathCalled);
}

TEST_F(TestOpticalHelper, IsDupFileNameInPath_DuplicateExists_ReturnsTrue)
{
    QString path = "/tmp/test";
    QUrl url("file:///tmp/test.txt");

    // Mock file info list to contain duplicate
    QFileInfo duplicateFile("/tmp/test/test.txt");
    mockFileInfoList.append(duplicateFile);

    bool result = OpticalHelper::isDupFileNameInPath(path, url);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, IsDupFileNameInPath_NoDuplicate_ReturnsFalse)
{
    QString path = "/tmp/test";
    QUrl url("file:///tmp/test.txt");

    // Mock file info list to not contain duplicate
    QFileInfo otherFile("/tmp/test/other.txt");
    mockFileInfoList.append(otherFile);

    bool result = OpticalHelper::isDupFileNameInPath(path, url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsDupFileNameInPath_InvalidInfo_ReturnsFalse)
{
    QString path = "/tmp/test";
    QUrl url("file:///tmp/test.txt");

    bool result = OpticalHelper::isDupFileNameInPath(path, url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsDupFileNameInPath_EmptyPath_ReturnsFalse)
{
    QString path = "";
    QUrl url("file:///tmp/test.txt");

    bool result = OpticalHelper::isDupFileNameInPath(path, url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsDupFileNameInPath_NonExistentDir_ReturnsFalse)
{
    QString path = "/tmp/nonexistent";
    QUrl url("file:///tmp/test.txt");

    mockDirExists = false;

    bool result = OpticalHelper::isDupFileNameInPath(path, url);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsBurnEnabled_ConfigTrue_ReturnsTrue)
{
    mockBurnEnabled = true;
    bool result = OpticalHelper::isBurnEnabled();

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalHelper, IsBurnEnabled_ConfigFalse_ReturnsFalse)
{
    mockBurnEnabled = false;
    bool result = OpticalHelper::isBurnEnabled();

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalHelper, IsBurnEnabled_InvalidConfig_ReturnsTrue)
{
    // Mock DConfigManager to return invalid QVariant
    stub.set_lamda(&DConfigManager::value, []() {
        __DBG_STUB_INVOKE__
        return QVariant();   // Invalid variant
    });

    bool result = OpticalHelper::isBurnEnabled();

    EXPECT_TRUE(result);   // Default to true when config is invalid
}

TEST_F(TestOpticalHelper, AllOpticalDiscMountPoints_ValidDiscs_ReturnsMountPoints)
{
    QStringList result = OpticalHelper::allOpticalDiscMountPoints();

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(mockMountPoint));
}

TEST_F(TestOpticalHelper, FindMountPoint_ValidPath_ReturnsMountPoint)
{
    QString path = "/media/disc/subfolder/test.txt";

    // Mock allOpticalDiscMountPoints
    stub.set_lamda(&OpticalHelper::allOpticalDiscMountPoints, [this]() {
        __DBG_STUB_INVOKE__
        return QStringList { mockMountPoint };
    });

    QString result = OpticalHelper::findMountPoint(path);

    EXPECT_EQ(result, mockMountPoint);
}

TEST_F(TestOpticalHelper, FindMountPoint_NoMatchingMountPoint_ReturnsEmpty)
{
    QString path = "/home/user/test.txt";

    // Mock allOpticalDiscMountPoints
    stub.set_lamda(&OpticalHelper::allOpticalDiscMountPoints, [this]() {
        __DBG_STUB_INVOKE__
        return QStringList { mockMountPoint };
    });

    QString result = OpticalHelper::findMountPoint(path);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOpticalHelper, IsTransparent_BurnSchemeOnDisc_ReturnsTrueWithCorrectStatus)
{
    QUrl url("burn:///dev/sr0/disc_files/test.txt");
    Global::TransparentStatus status;

    // Mock burnIsOnDisc to return true
    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = OpticalHelper::instance()->isTransparent(url, &status);

    EXPECT_TRUE(result);
    // Status should not be set to transparent for files on disc
}

TEST_F(TestOpticalHelper, IsTransparent_BurnSchemeInStaging_ReturnsTrueWithTransparentStatus)
{
    QUrl url("burn:///dev/sr0/staging_files/test.txt");
    Global::TransparentStatus status;

    // Mock burnIsOnDisc to return false
    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = OpticalHelper::instance()->isTransparent(url, &status);

    EXPECT_TRUE(result);
    EXPECT_EQ(status, Global::TransparentStatus::kTransparent);
}

TEST_F(TestOpticalHelper, IsTransparent_NonBurnScheme_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    Global::TransparentStatus status;

    bool result = OpticalHelper::instance()->isTransparent(url, &status);

    EXPECT_FALSE(result);
}

// Test private burnRxp function indirectly through public methods
TEST_F(TestOpticalHelper, BurnRxp_MatchesValidPaths)
{
    // Test through burnDestDevice which uses burnRxp
    QUrl validUrl("burn:///dev/sr0/disc_files/test.txt");
    QString device = OpticalHelper::burnDestDevice(validUrl);

    EXPECT_EQ(device, "/dev/sr0");
}

TEST_F(TestOpticalHelper, BurnRxp_RejectsInvalidPaths)
{
    // Test through burnDestDevice which uses burnRxp
    QUrl invalidUrl("burn:///invalid/format");
    QString device = OpticalHelper::burnDestDevice(invalidUrl);

    EXPECT_TRUE(device.isEmpty());
}
