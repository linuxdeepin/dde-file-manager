// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// NetworkGuard is header-only and only needs Qt6::Network, so these tests
// run without the plugin or a dde-shell process.
#include "networkguard.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>

TEST(NetworkGuardTest, LocalPathIsNeverBusy)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    EXPECT_FALSE(dock::NetworkGuard::instance().isBusy(dir.path()));
    EXPECT_FALSE(dock::NetworkGuard::instance().isNetworkPath(dir.path()));
    EXPECT_FALSE(dock::NetworkGuard::instance().isBusy(QString()));
}

TEST(NetworkGuardTest, ParsesGvfsMountNames)
{
    QString host;
    QStringList ports;

    const QString gvfsSmb = QStringLiteral("/run/user/1000/gvfs/smb-share:server=1.2.3.4,share=draw/sub");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(gvfsSmb, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("1.2.3.4"));
    EXPECT_EQ(ports, QStringList({ QStringLiteral("445"), QStringLiteral("139") }));

    const QString gvfsFtp = QStringLiteral("/run/user/1000/gvfs/ftp:host=ftp.example.com,port=2121/dir");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(gvfsFtp, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("ftp.example.com"));
    EXPECT_EQ(ports, QStringList({ QStringLiteral("2121") }));

    const QString gvfsSftp = QStringLiteral("/root/.gvfs/sftp:host=example.com/home/x");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(gvfsSftp, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("example.com"));
    EXPECT_EQ(ports, QStringList({ QStringLiteral("22") }));

    // Non-network GVFS mounts and plain home paths are not probed.
    EXPECT_FALSE(dock::NetworkGuard::parseHostPort(QStringLiteral("/run/user/1000/gvfs/http:host=web/dir"), &host, &ports));
    EXPECT_FALSE(dock::NetworkGuard::parseHostPort(QStringLiteral("/home/user/docs"), &host, &ports));
}

TEST(NetworkGuardTest, CifsMountPrefixIsSegmentBounded)
{
    QString host;
    QStringList ports;
    // A share whose name contains "smbmounts" must not let the prefix match
    // eat the share component (greedy [\s\S]* regression).
    const QString nested = QStringLiteral("/media/deepin/smbmounts/smb-share:server=1.2.3.4,share=smbmounts/data/file.txt");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(nested, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("1.2.3.4"));
    EXPECT_EQ(ports, QStringList({ QStringLiteral("445"), QStringLiteral("139") }));

    // Plain and /run/media variants keep resolving as before.
    const QString plain = QStringLiteral("/media/deepin/smbmounts/smb-share:server=1.2.3.4,share=draw/sub");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(plain, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("1.2.3.4"));

    const QString runMedia = QStringLiteral("/run/media/deepin/smbmounts/smb-share:server=example.com,share=draw/sub");
    ASSERT_TRUE(dock::NetworkGuard::parseHostPort(runMedia, &host, &ports));
    EXPECT_EQ(host, QStringLiteral("example.com"));

    // The prefix itself must not match a non-mount path segment.
    EXPECT_FALSE(dock::NetworkGuard::parseHostPort(QStringLiteral("/media/deepin/photos/smbmounts/"), &host, &ports));
}
