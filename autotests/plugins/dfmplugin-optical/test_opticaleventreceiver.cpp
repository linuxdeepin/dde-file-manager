// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "events/opticaleventreceiver.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QDir>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

class TestOpticalEventReceiver : public testing::Test
{
public:
    void SetUp() override
    {
        receiver = &OpticalEventReceiver::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    OpticalEventReceiver *receiver = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestOpticalEventReceiver, Instance_ReturnsSingleton)
{
    OpticalEventReceiver &instance1 = OpticalEventReceiver::instance();
    OpticalEventReceiver &instance2 = OpticalEventReceiver::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TestOpticalEventReceiver, HandleDeleteFilesShortcut_NonOpticalRoot_ReturnsFalse)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("file:///home/user/test.txt") };
    QUrl rootUrl = QUrl("file:///home/user");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = receiver->handleDeleteFilesShortcut(windowId, urls, rootUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleDeleteFilesShortcut_OpticalRootWithDiscFile_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("burn:///dev/sr0/ondisc/test.txt") };
    QUrl rootUrl = QUrl("burn:///dev/sr0");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->handleDeleteFilesShortcut(windowId, urls, rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandleDeleteFilesShortcut_OpticalRootWithPWSubDir_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("burn:///dev/sr0/staging/test.txt") };
    QUrl rootUrl = QUrl("burn:///dev/sr0");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(OpticalEventReceiver, isContainPWSubDirFile), [&](OpticalEventReceiver *obj, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->handleDeleteFilesShortcut(windowId, urls, rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandleCheckDragDropAction_EmptyUrls_ReturnsFalse)
{
    QList<QUrl> urls;
    QUrl urlTo("burn:///dev/sr0");
    Qt::DropAction action = Qt::CopyAction;

    bool result = receiver->handleCheckDragDropAction(urls, urlTo, &action);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleCheckDragDropAction_InvalidUrlTo_ReturnsFalse)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl urlTo;
    Qt::DropAction action = Qt::CopyAction;

    bool result = receiver->handleCheckDragDropAction(urls, urlTo, &action);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleCheckDragDropAction_BurnSchemeRootPath_ReturnsTrue)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl urlTo("burn:///dev/sr0");
    Qt::DropAction action = Qt::MoveAction;

    stub.set_lamda(&OpticalHelper::burnFilePath, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/");
    });

    bool result = receiver->handleCheckDragDropAction(urls, urlTo, &action);

    EXPECT_TRUE(result);
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(TestOpticalEventReceiver, HandleCheckDragDropAction_BurnSchemeNonRootPath_ReturnsFalse)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl urlTo("burn:///dev/sr0/subdir");
    Qt::DropAction action = Qt::CopyAction;

    stub.set_lamda(&OpticalHelper::burnFilePath, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/subdir");
    });

    bool result = receiver->handleCheckDragDropAction(urls, urlTo, &action);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleMoveToTrashShortcut_NonOpticalRoot_ReturnsFalse)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("file:///home/user/test.txt") };
    QUrl rootUrl = QUrl("file:///home/user");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = receiver->handleMoveToTrashShortcut(windowId, urls, rootUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleMoveToTrashShortcut_OpticalRootWithPWSubDir_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("burn:///dev/sr0/staging/test.txt") };
    QUrl rootUrl = QUrl("burn:///dev/sr0");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(OpticalEventReceiver, isContainPWSubDirFile), [&](OpticalEventReceiver *obj, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->handleMoveToTrashShortcut(windowId, urls, rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandleCutFilesShortcut_NonOpticalRoot_ReturnsFalse)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("file:///home/user/test.txt") };
    QUrl rootUrl = QUrl("file:///home/user");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = receiver->handleCutFilesShortcut(windowId, urls, rootUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleCutFilesShortcut_OpticalRootWithPWSubDir_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("burn:///dev/sr0/staging/test.txt") };
    QUrl rootUrl = QUrl("burn:///dev/sr0");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(OpticalEventReceiver, isContainPWSubDirFile), [&](OpticalEventReceiver *obj, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->handleCutFilesShortcut(windowId, urls, rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandlePasteFilesShortcut_NonOpticalPath_ReturnsFalse)
{
    quint64 windowId = 12345;
    QList<QUrl> fromUrls = { QUrl("file:///home/user/test.txt") };
    QUrl targetUrl = QUrl("file:///home/user");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = receiver->handlePasteFilesShortcut(windowId, fromUrls, targetUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandlePasteFilesShortcut_OpticalPathPWUserspace_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> fromUrls = { QUrl("file:///home/user/test.txt") };
    QUrl targetUrl = QUrl("burn:///dev/sr0/staging");

    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [&](DeviceProxyManager *obj, const QString &path) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = receiver->handlePasteFilesShortcut(windowId, fromUrls, targetUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, SepateTitlebarCrumb_NonBurnScheme_ReturnsFalse)
{
    QUrl url("file:///home/user/test.txt");
    QList<QVariantMap> mapGroup;

    bool result = receiver->sepateTitlebarCrumb(url, &mapGroup);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, SepateTitlebarCrumb_BurnScheme_ReturnsTrue)
{
    QUrl url = QUrl("burn:///dev/sr0/staging/test.txt");
    QList<QVariantMap> mapGroup;

    bool result = receiver->sepateTitlebarCrumb(url, &mapGroup);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandleDropFiles_BurnSchemeRootPath_ReturnsTrue)
{
    QList<QUrl> fromUrls = { QUrl("file:///home/user/test.txt") };
    QUrl targetUrl = QUrl("burn:///dev/sr0");

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [&](const QList<QUrl> &urls, QList<QUrl> *localUrls) {
        __DBG_STUB_INVOKE__
        if (localUrls) {
            *localUrls = urls;
        }
        return true;
    });

    bool result = receiver->handleDropFiles(fromUrls, targetUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, HandleDropFiles_NonBurnScheme_ReturnsFalse)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl toUrl("file:///home/user/target");

    bool result = receiver->handleDropFiles(fromUrls, toUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleBlockShortcutPaste_NonOpticalScheme_ReturnsFalse)
{
    quint64 windowId = 12345;
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl to("file:///home/user/target");

    stub.set_lamda(&OpticalHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("burn");
    });

    bool result = receiver->handleBlockShortcutPaste(windowId, fromUrls, to);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalEventReceiver, HandleBlockShortcutPaste_OpticalSchemeNonRootUrl_ReturnsTrue)
{
    quint64 windowId = 12345;
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/home/user/test.txt") };
    QUrl to("burn:///dev/sr0/subdir");

    stub.set_lamda(&OpticalHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("burn");
    });

    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::discRoot, [](const QString &dev) {
        __DBG_STUB_INVOKE__
        return QUrl("burn:///dev/sr0");
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return false;   // Non-root URL
    });

    bool result = receiver->handleBlockShortcutPaste(windowId, fromUrls, to);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, DetailViewIcon_OpticalSchemeRootUrl_ReturnsTrue)
{
    QUrl url("burn:///dev/sr0");
    QString iconName;

    stub.set_lamda(&OpticalHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("burn");
    });

    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::discRoot, [](const QString &dev) {
        __DBG_STUB_INVOKE__
        return QUrl("burn:///dev/sr0");
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return true;   // Root URL
    });

    bool result = receiver->detailViewIcon(url, &iconName);

    EXPECT_TRUE(result);
    EXPECT_EQ(iconName, "media-optical");
}

TEST_F(TestOpticalEventReceiver, HandleTabCloseable_StagingAndDiscSameDevice_ReturnsTrue)
{
    QUrl currentUrl("burn:///dev/sr0/staging/test");
    QUrl rootUrl("burn:///dev/sr0/ondisc");

    stub.set_lamda(&OpticalHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("burn");
    });

    stub.set_lamda(&OpticalHelper::burnIsOnStaging, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    bool result = receiver->handleTabCloseable(currentUrl, rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, IsContainPWSubDirFile_ContainsPWSubDir_ReturnsTrue)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/media/cdrom/subdir/test.txt") };

    stub.set_lamda(&OpticalHelper::findMountPoint, [](const QString &directory) {
        __DBG_STUB_INVOKE__
        return QString("/media/cdrom");
    });

    stub.set_lamda(&DeviceUtils::getMountInfo, [](const QString &mnt, bool withFilePath) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&DeviceUtils::isPWUserspaceOpticalDiscDev, [](const QString &dev) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->isContainPWSubDirFile(urls);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalEventReceiver, IsContainPWSubDirFile_RootDirectory_ReturnsFalse)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/media/cdrom/test.txt") };

    stub.set_lamda(&OpticalHelper::findMountPoint, [](const QString &directory) {
        __DBG_STUB_INVOKE__
        return QString("/media/cdrom");
    });

    bool result = receiver->isContainPWSubDirFile(urls);

    EXPECT_FALSE(result);
}
