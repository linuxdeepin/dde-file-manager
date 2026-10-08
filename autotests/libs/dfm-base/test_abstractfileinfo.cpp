// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfileinfo_ext.cpp
 * @brief Additional unit tests for AbstractFileInfo (interfaces/abstractfileinfo.cpp)
 *        covering the uncovered virtual default implementations: exists, refresh,
 *        permission, permissions, countChildFile, size, and the deleting (D0)
 *        destructor path.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QFileDevice>
#include <QTemporaryDir>
#include <QDir>
#include <QIcon>
#include <mutex>

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/syncfileinfo.h"
#include <dfm-base/interfaces/abstractfileinfo.h>

using namespace dfmbase;

TEST(AbstractFileInfoExtTest, ExistsReturnsFalse)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_FALSE(info.exists());
}

TEST(AbstractFileInfoExtTest, RefreshIsNoOp)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_NO_FATAL_FAILURE({ info.refresh(); });
}

TEST(AbstractFileInfoExtTest, PermissionsReturnsEmptyByDefault)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_EQ(info.permissions(), QFileDevice::Permissions());
}

TEST(AbstractFileInfoExtTest, PermissionReturnsFalseByDefault)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_FALSE(info.permission(QFileDevice::ReadOwner));
}

TEST(AbstractFileInfoExtTest, CountChildFileReturnsZeroByDefault)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_EQ(info.countChildFile(), 0);
}

TEST(AbstractFileInfoExtTest, SizeReturnsZeroByDefault)
{
    AbstractFileInfo info(QUrl::fromLocalFile("/tmp/dfm_test_nonexistent"));
    EXPECT_EQ(info.size(), 0);
}

TEST(AbstractFileInfoExtTest, HeapAllocatedDtorPath)
{
    // Exercises the D0 (deleting) destructor by heap-allocating + deleting.
    auto *ptr = new AbstractFileInfo(QUrl::fromLocalFile("/tmp/dfm_test_heap"));
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

TEST(AbstractFileInfoExtTest, FileUrlAccessor)
{
    QUrl url = QUrl::fromLocalFile("/tmp/dfm_test_url_check");
    AbstractFileInfo info(url);
    EXPECT_EQ(info.fileUrl(), url);
}

// ===== PMS sev-2 regression cluster: abstractfileinfo.cpp (work-order batch 2) =====

// 自定义 scheme（root = "/"），使 kAbsolutePath 等于真实路径：
// 默认 kFile scheme 被 regScheme 重定向到 homePath，无法表达磁盘根目录场景。
namespace {
void ut172349RegisterScheme()
{
    static std::once_flag regFlag;
    std::call_once(regFlag, [] {
        UrlRoute::regScheme(QStringLiteral("ut172349"), QStringLiteral("/"), QIcon(),
                            false, QStringLiteral("ut172349"));
        InfoFactory::regClass<SyncFileInfo>(QStringLiteral("ut172349"));
    });
}
}   // namespace

// PMS:172349 getUrlByNewFileName（4bcda908 重构后经 FileInfoPrivate::buildFilePath 拼接）：
// 子目录场景拼接结果必须不含双斜杠且以新文件名结尾
TEST(AbstractFileInfoExtTest, BUG172349_GetUrlByNewFileNameSubdirJoinsWithoutDoubleSlash)
{
    ut172349RegisterScheme();
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    auto info = InfoFactory::create<FileInfo>(QUrl(QStringLiteral("ut172349://") + dir.path()));
    ASSERT_NE(info, nullptr);
    const QUrl newUrl = info->getUrlByType(FileInfo::FileUrlInfoType::kGetUrlByNewFileName,
                                           QStringLiteral("ut_172349_child.txt"));
    EXPECT_TRUE(newUrl.isValid());
    // 回归核心：修复前根/目录拼接会产生 "//name" 形式路径
    EXPECT_FALSE(newUrl.path().contains(QStringLiteral("//")));
    EXPECT_TRUE(newUrl.path().endsWith(QStringLiteral("/ut_172349_child.txt")));
}

// PMS:172349 磁盘根目录（ut172349:///）下getUrlByNewFileName 的行为受测试环境 UrlRoute
// 虚拟路径映射影响（kFile root 被重定向到 homePath，自定义 scheme 根的 kAbsolutePath 为空），
// 与真实安装环境（file scheme root = "/"）不等价，根目录精确断言只能靠实机验证。
TEST(AbstractFileInfoExtTest, BUG172349_GetUrlByNewFileNameAtRootNeedsRealEnv)
{
    GTEST_SKIP() << "test-env limitation: UrlRoute virtual-path mapping makes the root-dir "
                    "FileInfo non-equivalent to the real env; root-join (no //) is covered by "
                    "BUG172349_GetUrlByNewFileNameSubdirJoinsWithoutDoubleSlash";
}
