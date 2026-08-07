// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/utils/loggerrules.h>
#include <dfm-base/utils/signalhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/file/local/localfilewatcher.h>

using namespace dfmbase;

namespace {
class FakeBasePreview : public AbstractBasePreview
{
public:
    FakeBasePreview() : AbstractBasePreview() {}
    bool setFileUrl(const QUrl &) override { return false; }
    QUrl fileUrl() const override { return QUrl(); }
    QWidget *contentWidget() const override { return nullptr; }
};
}   // namespace

TEST(BulkTest, AbstractFileWatcherDefaults)
{
    LocalFileWatcher w(QUrl::fromLocalFile("/tmp/test"));
    EXPECT_NO_FATAL_FAILURE({ (void)w.url(); });
    EXPECT_NO_FATAL_FAILURE({ (void)w.startWatcher(); });
    EXPECT_NO_FATAL_FAILURE({ (void)w.stopWatcher(); });
    EXPECT_NO_FATAL_FAILURE({ (void)w.restartWatcher(); });
    EXPECT_NO_FATAL_FAILURE({ w.setEnabledSubfileWatcher(QUrl("file:///tmp/a"), true); });
    EXPECT_NO_FATAL_FAILURE({ w.notifyFileAdded(QUrl("file:///tmp/x")); });
    EXPECT_NO_FATAL_FAILURE({ w.notifyFileChanged(QUrl("file:///tmp/x")); });
}

TEST(BulkTest, AbstractBasePreviewDefaults)
{
    FakeBasePreview p;
    EXPECT_NO_FATAL_FAILURE({ (void)p.statusBarWidget(); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.statusBarWidgetAlignment(); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.title(); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.showStatusBarSeparator(); });
    EXPECT_NO_FATAL_FAILURE({ p.play(); });
    EXPECT_NO_FATAL_FAILURE({ p.pause(); });
    EXPECT_NO_FATAL_FAILURE({ p.stop(); });
    EXPECT_NO_FATAL_FAILURE({ p.handleBeforDestroy(); });
}

TEST(BulkTest, ProxyFileInfoConstruct)
{
    ProxyFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_proxy"));
    SUCCEED();
}

TEST(BulkTest, ClipboardSetUrl)
{
    ClipBoard *cb = ClipBoard::instance();
    cb->setUrlsToClipboard({}, ClipBoard::ClipboardAction::kCopyAction);
    SUCCEED();
}

TEST(BulkTest, LoggerRulesConstruct)
{
    LoggerRules rules;
    SUCCEED();
}

TEST(BulkTest, SignalHandlerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)SignalHandler::instance(); });
}

TEST(BulkTest, DeviceProxyManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance(); });
}

TEST(BulkTest, DeviceProxyManagerIsDBusRuning)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->isDBusRuning(); });
}

TEST(BulkTest, DeviceProxyManagerIsFileOfExternalMounts)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->isFileOfExternalMounts("/tmp/test.txt"); });
}

TEST(BulkTest, DeviceProxyManagerIsFileOfProtocolMounts)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->isFileOfProtocolMounts("/tmp/test.txt"); });
}

TEST(BulkTest, DeviceProxyManagerIsFileOfExternalBlockMounts)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->isFileOfExternalBlockMounts("/tmp/test.txt"); });
}

TEST(BulkTest, DeviceProxyManagerIsMptOfDevice)
{
    QString id;
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->isMptOfDevice("/tmp", id); });
}

TEST(BulkTest, DeviceProxyManagerQueryDeviceInfoByPath)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceProxyManager::instance()->queryDeviceInfoByPath("/tmp", false); });
}

TEST(BulkTest, DeviceProxyManagerSubscribeUsageMonitoring)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceProxyManager::instance()->subscribeUsageMonitoring(); });
}

TEST(BulkTest, DeviceProxyManagerUnsubscribeUsageMonitoring)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceProxyManager::instance()->unsubscribeUsageMonitoring(); });
}

TEST(BulkTest, DeviceProxyManagerRefreshUsage)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceProxyManager::instance()->refreshUsage(); });
}

TEST(BulkTest, DeviceProxyManagerReloadOpticalInfo)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceProxyManager::instance()->reloadOpticalInfo("/dev/sr0"); });
}
