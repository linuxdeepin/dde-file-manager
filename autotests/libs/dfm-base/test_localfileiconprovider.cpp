// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfileiconprovider.cpp
 * @brief Unit tests for LocalFileIconProvider (localfileiconprovider.cpp)
 *
 * LocalFileIconProvider wraps QFileIconProvider with DFMIO-based icon
 * resolution. The icon() overloads resolve a QIcon from the file system;
 * when the path does not exist (or has no themed icon) they return a null
 * QIcon, and the feedback overloads fall back to the caller-supplied icon.
 * All paths here are local temp files / non-existent paths, so no hardware
 * or network is required.
 */

#include <gtest/gtest.h>
#include <dfm-base/file/local/localfileiconprovider.h>

#include <QFileInfo>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QString>
#include <QFile>

using namespace dfmbase;

namespace {
// Build a guaranteed non-null QIcon (an offscreen-rendered pixmap) so that
// feedback assertions are not dependent on the current icon theme.
QIcon utNonNullIcon()
{
    QPixmap pm(16, 16);
    pm.fill(Qt::red);
    return QIcon(pm);
}
}   // namespace

TEST(LocalFileIconProviderTest, GlobalProviderReturnsNonNullSingleton)
{
    auto *a = LocalFileIconProvider::globalProvider();
    auto *b = LocalFileIconProvider::globalProvider();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

TEST(LocalFileIconProviderTest, ConstructAndDestructWithoutCrash)
{
    {
        LocalFileIconProvider provider;
        (void)provider;
    }
    SUCCEED();
}

TEST(LocalFileIconProviderTest, IconForNonExistentPathByQStringReturnsNullIcon)
{
    LocalFileIconProvider provider;
    QIcon icon = provider.icon(QStringLiteral("/this/path/does/not/exist/ut_lfip_12345"));
    EXPECT_TRUE(icon.isNull());
}

TEST(LocalFileIconProviderTest, IconForNonExistentPathByQFileInfoReturnsNullIcon)
{
    LocalFileIconProvider provider;
    QFileInfo info(QStringLiteral("/this/path/does/not/exist/ut_lfip_67890"));
    QIcon icon = provider.icon(info);
    EXPECT_TRUE(icon.isNull());
}

TEST(LocalFileIconProviderTest, IconByQStringWithFeedbackFallsBackWhenIconNull)
{
    LocalFileIconProvider provider;
    QIcon feedback = utNonNullIcon();
    ASSERT_FALSE(feedback.isNull());
    QIcon result = provider.icon(QStringLiteral("/this/path/does/not/exist/ut_lfip_fb1"), feedback);
    // Non-existent path -> primary icon null -> must return the feedback icon.
    EXPECT_FALSE(result.isNull());
}

TEST(LocalFileIconProviderTest, IconByQFileInfoWithFeedbackFallsBackWhenIconNull)
{
    LocalFileIconProvider provider;
    QFileInfo info(QStringLiteral("/this/path/does/not/exist/ut_lfip_fb2"));
    QIcon feedback = utNonNullIcon();
    ASSERT_FALSE(feedback.isNull());
    QIcon result = provider.icon(info, feedback);
    EXPECT_FALSE(result.isNull());
}

TEST(LocalFileIconProviderTest, IconForExistingTempFileDoesNotCrash)
{
    QTemporaryFile tmp(QDir::tempPath() + "/ut_lfip_XXXXXX.txt");
    ASSERT_TRUE(tmp.open());
    tmp.write("x");
    tmp.close();
    QString path = tmp.fileName();   // keep file alive until end of test

    LocalFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo(path));
    // The parent dir exists, so fileSystemIcon runs the "exists" path; the
    // returned QIcon may or may not be null depending on the icon theme, but
    // the call must not crash.
    (void)icon.isNull();
    QFile::remove(path);
}
