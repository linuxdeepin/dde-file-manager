// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocolutils_pms.cpp
 * @brief PMS sev-2 regression tests for ProtocolUtils
 *        (src/dfm-base/utils/protocolutils.cpp).
 *
 * Bug -> case mapping (fix commits verified via `git show`):
 *   - PMS:303519 nfs/gvfs 路径识别：gvfs 挂载的 nfs 目录（老式
 *     "~/.gvfs/nfs:..." 与新式 "/run/user/<uid>/gvfs/nfs:..."）必须
 *     被 isNFSFile 识别为 NFS 文件。
 *   - PMS:331315 dav/davs 识别与手动通知：isDavFile/isDavsFile 需按
 *     gvfs 路径中的 ssl 标记区分 dav 与 davs，且对 nfs url 手动触发
 *     文件变更通知必须安全（notifyFileChangeManual 无监听时直接返回）。
 */

#include <gtest/gtest.h>
#include <QUrl>

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

// PMS:303519 gvfs 挂载的 nfs 路径必须被识别为 NFS 文件
TEST(ProtocolUtilsPmsTest, BUG303519_NfsGvfsPathsRecognized)
{
    // New style gvfs mount root.
    EXPECT_TRUE(ProtocolUtils::isNFSFile(
            QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs/nfs:host=server,path=/export/data"))));
    // Legacy gvfs mount root.
    EXPECT_TRUE(ProtocolUtils::isNFSFile(
            QUrl::fromLocalFile(QStringLiteral("/root/.gvfs/nfs:host=server,path=/export/data"))));
    // nfs scheme url.
    EXPECT_TRUE(ProtocolUtils::isNFSFile(QUrl(QStringLiteral("nfs://server/path"))));

    // Ordinary local paths are not nfs.
    EXPECT_FALSE(ProtocolUtils::isNFSFile(QUrl::fromLocalFile(QStringLiteral("/home/user/dir/nfs-not-real"))));
}

// PMS:331315 dav/davs 按 ssl 标记区分且手动通知安全
TEST(ProtocolUtilsPmsTest, BUG331315_DavDavsSslFlagAndManualNotifySafety)
{
    // dav (ssl=false) vs davs (ssl=true) inside gvfs paths.
    const QUrl davPath = QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs/dav:host=server,ssl=false"));
    const QUrl davsPath = QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs/dav:host=server,ssl=true"));
    EXPECT_TRUE(ProtocolUtils::isDavFile(davPath));
    EXPECT_FALSE(ProtocolUtils::isDavsFile(davPath));
    EXPECT_TRUE(ProtocolUtils::isDavsFile(davsPath));
    EXPECT_FALSE(ProtocolUtils::isDavFile(davsPath));
    EXPECT_TRUE(ProtocolUtils::isDavFile(QUrl(QStringLiteral("dav://server/path"))));
    EXPECT_TRUE(ProtocolUtils::isDavsFile(QUrl(QStringLiteral("davs://server/path"))));

    // Manual change notification for a gvfs nfs url is safe even when
    // no file watcher/listener exists for the parent directory.
    const QUrl nfsUrl = QUrl::fromLocalFile(
            QStringLiteral("/run/user/1000/gvfs/nfs:host=server,path=/export/data/file.txt"));
    EXPECT_NO_FATAL_FAILURE({
        FileUtils::notifyFileChangeManual(Global::FileNotifyType::kFileAdded, nfsUrl);
    });
}
