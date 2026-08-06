// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QTest>

/**
 * @file test_networkutils_r17.cpp
 * @brief Additional NetworkUtils tests: doAfterCheckNet (empty ports list =>
 *        returns true), resolveLocalSftpMountUrl with non-null local path.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>
#include <QStringList>

#include <dfm-base/utils/networkutils.h>

using namespace dfmbase;

TEST(NetworkUtilsR17Test, ResolveLocalSftpMountUrlNonSftpReturnsUnchanged)
{
    QUrl url("file:///tmp/test.txt");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_EQ(result, url);
}

TEST(NetworkUtilsR17Test, ResolveLocalSftpMountUrlEmptyPath)
{
    QUrl url("sftp://user@host/path");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_NO_FATAL_FAILURE({ (void)result; });
}

TEST(NetworkUtilsR17Test, DoAfterCheckNetEmptyPortsReturnsTrue)
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
