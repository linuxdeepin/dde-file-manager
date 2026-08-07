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
