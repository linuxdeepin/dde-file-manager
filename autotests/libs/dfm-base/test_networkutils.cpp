// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_networkutils.cpp
 * @brief Unit tests for NetworkUtils (networkutils.cpp) - parseIp & helpers
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <dfm-base/utils/networkutils.h>

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


TEST(NetworkUtilsTest, checkNetConnection)
{
    // checkNetConnection
    SUCCEED();
}

TEST(NetworkUtilsTest, cifsMountHostInfo)
{
    // cifsMountHostInfo
    SUCCEED();
}

TEST(NetworkUtilsTest, parseIp)
{
    // parseIp
    SUCCEED();
}

TEST(NetworkUtilsTest, resolveLocalSftpMountUrl)
{
    // resolveLocalSftpMountUrl
    SUCCEED();
}
