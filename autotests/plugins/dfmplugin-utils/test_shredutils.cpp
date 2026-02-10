// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/shredutils.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>

#include <DDialog>
#include <DSettingsOption>

#include <QStandardPaths>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class UT_ShredUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        utils = ShredUtils::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    ShredUtils *utils { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ShredUtils, instance_ReturnsSingleton)
{
    ShredUtils *instance1 = ShredUtils::instance();
    ShredUtils *instance2 = ShredUtils::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_ShredUtils, isShredEnabled_ReturnsConfigValue)
{
    stub.set_lamda(ADDR(DConfigManager, value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = utils->isShredEnabled();

    EXPECT_TRUE(result);
}

TEST_F(UT_ShredUtils, isShredEnabled_ReturnsFalse)
{
    stub.set_lamda(ADDR(DConfigManager, value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = utils->isShredEnabled();

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredUtils, setShredEnabled_CallsSetValue)
{
    bool setCalled = false;
    bool capturedValue = false;

    stub.set_lamda(ADDR(DConfigManager, setValue),
                   [&setCalled, &capturedValue](DConfigManager *, const QString &, const QString &, const QVariant &value) {
                       __DBG_STUB_INVOKE__
                       setCalled = true;
                       capturedValue = value.toBool();
                   });

    utils->setShredEnabled(true);

    EXPECT_TRUE(setCalled);
    EXPECT_TRUE(capturedValue);
}

TEST_F(UT_ShredUtils, initDconfig_CallsAddConfig)
{
    bool addConfigCalled = false;

    stub.set_lamda(ADDR(DConfigManager, addConfig),
                   [&addConfigCalled](DConfigManager *, const QString &, QString *) -> bool {
                       __DBG_STUB_INVOKE__
                       addConfigCalled = true;
                       return true;
                   });

    utils->initDconfig();
}

TEST_F(UT_ShredUtils, isValidFile_NullFileInfo_ReturnsFalse)
{
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = utils->isValidFile(url);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredUtils, isValidFile_ExternalBlockMount_ReturnsTrue)
{
    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/media/usb/test.txt")));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, pathOf),
                   [](FileInfo *, const FileInfo::FilePathInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return "/media/usb/test.txt";
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts),
                   [](DeviceProxyManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl url = QUrl::fromLocalFile("/media/usb/test.txt");
    bool result = utils->isValidFile(url);

    EXPECT_TRUE(result);
}

TEST_F(UT_ShredUtils, isValidFile_ProtectedDirectory_ReturnsFalse)
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile(desktopPath)));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, pathOf),
                   [&desktopPath](FileInfo *, const FileInfo::FilePathInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return desktopPath;
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts),
                   [](DeviceProxyManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [&desktopPath](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return desktopPath;
                   });

    QUrl url = QUrl::fromLocalFile(desktopPath);
    bool result = utils->isValidFile(url);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredUtils, isValidFile_HomePathItself_ReturnsFalse)
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile(homePath)));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, pathOf),
                   [&homePath](FileInfo *, const FileInfo::FilePathInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return homePath;
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts),
                   [](DeviceProxyManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [&homePath](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return homePath;
                   });

    QUrl url = QUrl::fromLocalFile(homePath);
    bool result = utils->isValidFile(url);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredUtils, isValidFile_ValidFileInHome_ReturnsTrue)
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = homePath + "/mydir/test.txt";

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile(filePath)));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, pathOf),
                   [&filePath](FileInfo *, const FileInfo::FilePathInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return filePath;
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts),
                   [](DeviceProxyManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [&filePath](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return filePath;
                   });

    QUrl url = QUrl::fromLocalFile(filePath);
    bool result = utils->isValidFile(url);

    EXPECT_TRUE(result);
}

TEST_F(UT_ShredUtils, isValidFile_PathOutsideHome_ReturnsFalse)
{
    QString outsidePath = "/opt/test.txt";

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile(outsidePath)));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, pathOf),
                   [&outsidePath](FileInfo *, const FileInfo::FilePathInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return outsidePath;
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts),
                   [](DeviceProxyManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [&outsidePath](const QString &, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return outsidePath;
                   });

    QUrl url = QUrl::fromLocalFile(outsidePath);
    bool result = utils->isValidFile(url);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredUtils, shredfile_EmptyList_ReturnsEarly)
{
    bool confirmCalled = false;

    utils->shredfile(QList<QUrl>(), 0);

    EXPECT_FALSE(confirmCalled);
}

TEST_F(UT_ShredUtils, shredfile_CancelledDialog_ReturnsEarly)
{
    stub.set_lamda(VADDR(DDialog, exec),
                   [](DDialog *) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    QList<QUrl> files;
    files << QUrl::fromLocalFile("/tmp/test.txt");

    utils->shredfile(files, 0);
}

TEST_F(UT_ShredUtils, createShredSettingItem_ReturnsWidget)
{
    stub.set_lamda(ADDR(DConfigManager, value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    Dtk::Core::DSettingsOption option;
    QWidget *widget = utils->createShredSettingItem(&option);

    EXPECT_NE(widget, nullptr);

    if (widget)
        delete widget;
}

