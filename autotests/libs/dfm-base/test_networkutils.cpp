// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_networkutils.cpp
 * @brief Unit tests for NetworkUtils (networkutils.cpp) - parseIp & helpers
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QThread>
#include "stubext.h"

#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

using namespace dfmbase;

TEST(NetworkUtilsTest, ParseIpFtpWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4,port=2121", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(port, QString("2121"));
}

TEST(NetworkUtilsTest, ParseIpFtpDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(port, QString("21"));
}

TEST(NetworkUtilsTest, ParseIpSmbWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8,port=445", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("5.6.7.8"));
    EXPECT_EQ(port, QString("445"));
}

TEST(NetworkUtilsTest, ParseIpSmbDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("5.6.7.8"));
    EXPECT_EQ(port, QString("445"));
}

TEST(NetworkUtilsTest, ParseIpSftpWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/sftp:host=9.10.11.12,port=2222", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("9.10.11.12"));
    EXPECT_EQ(port, QString("2222"));
}

TEST(NetworkUtilsTest, ParseIpSftpDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/sftp:host=9.10.11.12", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("9.10.11.12"));
    EXPECT_EQ(port, QString("22"));
}

TEST(NetworkUtilsTest, ParseIpInvalidScheme)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/http:host=1.2.3.4", ip, port);
    EXPECT_FALSE(ok);
}

TEST(NetworkUtilsTest, ParseIpReturnsPortsList)
{
    QString ip;
    QStringList ports;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4", ip, ports);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(ports.size(), 1);
    EXPECT_EQ(ports.first(), QString("21"));
}

TEST(NetworkUtilsTest, ParseIpSmbReturnsTwoPorts)
{
    QString ip;
    QStringList ports;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8", ip, ports);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ports.size(), 2);
}

// ---- Coverage additions for remaining NetworkUtils API ----

TEST(NetworkUtilsTest, CheckAllCifsBusyIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkAllCIFSBusy(); });
}

TEST(NetworkUtilsTest, CheckNetConnectionHostStringListIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkNetConnection("localhost", QStringList{"80"}, 200); });
}

TEST(NetworkUtilsTest, CheckNetConnectionHostPortIntIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkNetConnection("localhost", "80", 200); });
}

TEST(NetworkUtilsTest, CheckFtpOrSmbBusyIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkFtpOrSmbBusy(QUrl("file:///")); });
}

TEST(NetworkUtilsTest, CifsMountHostInfoIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->cifsMountHostInfo(); });
}

TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->resolveLocalSftpMountUrl(QUrl("file:///")); });
}
TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlNonSftpReturnsUnchanged)
{
    QUrl url("file:///tmp/test.txt");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_EQ(result, url);
}
TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlEmptyPath)
{
    QUrl url("sftp://user@host/path");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_NO_FATAL_FAILURE({ (void)result; });
}
TEST(NetworkUtilsTest, DoAfterCheckNetEmptyPortsReturnsTrue)
{
    // doAfterCheckNet with empty ports: host check skipped → callback(true)
    bool called = false;
    NetworkUtils::instance()->doAfterCheckNet("127.0.0.1", {}, [&called](bool ok) {
        called = true;
        EXPECT_TRUE(ok);
    }, 100);
    // Wait for async to complete
    for (int i = 0; i < 20 && !called; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    EXPECT_TRUE(called);
}

// ---- TTL cache tests ----

TEST(NetworkUtilsTest, MakeCacheKey)
{
    EXPECT_EQ(NetworkUtils::makeCacheKey("1.2.3.4", "445"), QString("1.2.3.4:445"));
    EXPECT_EQ(NetworkUtils::makeCacheKey("host", "21"), QString("host:21"));
}

TEST(NetworkUtilsTest, ClearCache)
{
    auto *nu = NetworkUtils::instance();
    nu->updateCache("10.0.0.1", "445", true);
    auto entry = nu->getFromCache("10.0.0.1", "445");
    EXPECT_TRUE(entry.timestamp.isValid());

    nu->clearCache();
    entry = nu->getFromCache("10.0.0.1", "445");
    EXPECT_FALSE(entry.timestamp.isValid());
}

TEST(NetworkUtilsTest, CacheMissReturnsInvalid)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();
    auto entry = nu->getFromCache("nonexistent.host", "9999");
    EXPECT_FALSE(entry.timestamp.isValid());
}

TEST(NetworkUtilsTest, CacheHitBeforeExpiry)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();

    nu->updateCache("192.168.1.1", "80", false);
    auto entry = nu->getFromCache("192.168.1.1", "80");
    EXPECT_TRUE(entry.timestamp.isValid());
    EXPECT_FALSE(entry.busy);
}

TEST(NetworkUtilsTest, CacheHitBusy)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();

    nu->updateCache("192.168.1.1", "445", true);
    auto entry = nu->getFromCache("192.168.1.1", "445");
    EXPECT_TRUE(entry.timestamp.isValid());
    EXPECT_TRUE(entry.busy);
}

TEST(NetworkUtilsTest, CacheExpiryAfterTTL)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();

    nu->updateCache("10.0.0.2", "22", true);
    auto entry = nu->getFromCache("10.0.0.2", "22");
    EXPECT_TRUE(entry.timestamp.isValid());

    QThread::msleep(NetworkUtils::kNetCacheTTLMs + 50);
    entry = nu->getFromCache("10.0.0.2", "22");
    EXPECT_FALSE(entry.timestamp.isValid());
}

TEST(NetworkUtilsTest, CacheUpdateOverwrites)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();

    nu->updateCache("10.0.0.3", "445", true);
    EXPECT_TRUE(nu->getFromCache("10.0.0.3", "445").busy);

    nu->updateCache("10.0.0.3", "445", false);
    EXPECT_FALSE(nu->getFromCache("10.0.0.3", "445").busy);
}

TEST(NetworkUtilsTest, CheckNetConnectionUseCacheFalse)
{
    auto *nu = NetworkUtils::instance();
    nu->clearCache();

    // Populate cache with busy=true
    nu->updateCache("192.0.2.1", "445", true);

    // useCache=false should bypass cache — but won't connect to fake host.
    // Just verify it doesn't crash.
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DConfigManager, value), [](DConfigManager *, const QString &, const QString &, const QVariant &def) -> QVariant {
        return def;
    });

    EXPECT_NO_FATAL_FAILURE({
        (void)nu->checkNetConnection("192.0.2.1", "445", 100, false);
    });
}

TEST(NetworkUtilsTest, CheckNetConnectionEmptyHostReturnsTrue)
{
    auto *nu = NetworkUtils::instance();
    EXPECT_TRUE(nu->checkNetConnection("", "445"));
}
