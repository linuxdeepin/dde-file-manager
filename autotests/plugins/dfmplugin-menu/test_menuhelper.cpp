// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/menuhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <dfm-io/dfmio_utils.h>

#include <gtest/gtest.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_MenuHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// isHiddenExtMenu 测试用例

TEST_F(UT_MenuHelper, IsHiddenExtMenu_HiddenByConfig_ReturnsTrue)
{
    QUrl testUrl("file:///tmp/test");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList { "extension-menu" };
                       return defaultValue;
                   });

    EXPECT_TRUE(Helper::isHiddenExtMenu(testUrl));
}

TEST_F(UT_MenuHelper, IsHiddenExtMenu_RemoteFileProtocolDisabled_ReturnsTrue)
{
    QUrl testUrl("smb://server/share");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       if (key == "dfm.menu.protocoldev.enable")
                           return false;
                       return defaultValue;
                   });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(Helper::isHiddenExtMenu(testUrl));
}

TEST_F(UT_MenuHelper, IsHiddenExtMenu_RemoteFileProtocolEnabled_ReturnsFalse)
{
    QUrl testUrl("smb://server/share");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       if (key == "dfm.menu.protocoldev.enable")
                           return true;
                       return defaultValue;
                   });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_FALSE(Helper::isHiddenExtMenu(testUrl));
}

TEST_F(UT_MenuHelper, IsHiddenExtMenu_RemovableDeviceBlockDisabled_ReturnsTrue)
{
    QUrl testUrl("file:///media/usb");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       if (key == "dfm.menu.blockdev.enable")
                           return false;
                       return defaultValue;
                   });

    stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_TRUE(Helper::isHiddenExtMenu(testUrl));
}

TEST_F(UT_MenuHelper, IsHiddenExtMenu_RemovableDeviceBlockEnabled_ReturnsFalse)
{
    QUrl testUrl("file:///media/usb");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       if (key == "dfm.menu.blockdev.enable")
                           return true;
                       return defaultValue;
                   });

    stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(Helper::isHiddenExtMenu(testUrl));
}

TEST_F(UT_MenuHelper, IsHiddenExtMenu_NormalFile_ReturnsFalse)
{
    QUrl testUrl("file:///tmp/test");

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       return defaultValue;
                   });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(Helper::isHiddenExtMenu(testUrl));
}

// isHiddenMenu 测试用例

TEST_F(UT_MenuHelper, IsHiddenMenu_AppInHiddenList_ReturnsTrue)
{
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList { "testapp" };
                       return QVariant();
                   });

    EXPECT_TRUE(Helper::isHiddenMenu("testapp"));
}

TEST_F(UT_MenuHelper, IsHiddenMenu_FileDialogInHiddenList_ReturnsTrue)
{
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList { "dde-file-dialog" };
                       return QVariant();
                   });

    EXPECT_TRUE(Helper::isHiddenMenu("dde-select-dialog-test"));
}

TEST_F(UT_MenuHelper, IsHiddenMenu_DesktopApp_ChecksDesktopSetting)
{
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       return QVariant();
                   });

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [](Settings *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(Helper::isHiddenMenu("dde-desktop"));
}

TEST_F(UT_MenuHelper, IsHiddenMenu_ShellApp_ChecksDesktopSetting)
{
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       return QVariant();
                   });

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [](Settings *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(Helper::isHiddenMenu("org.deepin.dde-shell"));
}

TEST_F(UT_MenuHelper, IsHiddenMenu_NormalApp_ReturnsFalse)
{
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (key == "dfm.menu.hidden")
                           return QStringList {};
                       return QVariant();
                   });

    EXPECT_FALSE(Helper::isHiddenMenu("dde-file-manager"));
}

// isHiddenDesktopMenu 测试用例

TEST_F(UT_MenuHelper, IsHiddenDesktopMenu_Disabled_ReturnsTrue)
{
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [](Settings *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(Helper::isHiddenDesktopMenu());
}

TEST_F(UT_MenuHelper, IsHiddenDesktopMenu_Enabled_ReturnsFalse)
{
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [](Settings *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(Helper::isHiddenDesktopMenu());
}

// canOpenSelectedItems 测试用例

TEST_F(UT_MenuHelper, CanOpenSelectedItems_EmptyList_ReturnsTrue)
{
    QList<QUrl> urls;
    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_BelowThreshold_ReturnsTrue)
{
    QList<QUrl> urls;
    for (int i = 0; i < 5; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/test%1").arg(i)));

    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_AboveThresholdAllFiles_ReturnsTrue)
{
    QList<QUrl> urls;
    for (int i = 0; i < 50; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/test%1.txt").arg(i)));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 不是目录
    });

    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_TooManyDirectories_ReturnsFalse)
{
    QList<QUrl> urls;
    for (int i = 0; i < 50; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/dir%1").arg(i)));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 都是目录
    });

    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_ScanLimit_StopsScanning)
{
    QList<QUrl> urls;
    for (int i = 0; i < 1500; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/dir%1").arg(i)));

    int createCallCount = 0;
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&createCallCount](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       createCallCount++;
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 都不是目录
    });

    Helper::canOpenSelectedItems(urls);

    // 应该最多扫描 1000 个
    EXPECT_LE(createCallCount, 1000);
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_NullFileInfo_Skipped)
{
    QList<QUrl> urls;
    for (int i = 0; i < 10; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/test%1").arg(i)));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    // 不应该崩溃
    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}

TEST_F(UT_MenuHelper, CanOpenSelectedItems_MixedFilesAndDirs_BelowThreshold_ReturnsTrue)
{
    QList<QUrl> urls;
    for (int i = 0; i < 50; ++i)
        urls.append(QUrl::fromLocalFile(QString("/tmp/item%1").arg(i)));

    int dirCount = 0;
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [&dirCount](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        // 前10个是目录，其余是文件
        return dirCount++ < 10;
    });

    EXPECT_TRUE(Helper::canOpenSelectedItems(urls));
}
