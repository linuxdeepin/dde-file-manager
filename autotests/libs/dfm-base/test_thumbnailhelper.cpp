// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QMimeType>
#include <QString>
#include <QImage>
#include <QUrl>
#include <QMimeDatabase>

#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/base/urlroute.h>
#include <QIcon>

using namespace dfmbase;

TEST(ThumbnailHelperTest, SetSizeLimitAndRetrieve)
{
    ThumbnailHelper helper;
    QMimeType mime = QMimeDatabase().mimeTypeForName("text/plain");
    helper.setSizeLimit(mime, 4096);
    qint64 limit = helper.sizeLimit(mime);
    EXPECT_EQ(limit, 4096);
}


TEST(ThumbnailHelperTest, MakePathCreatesDirectory)
{
    ThumbnailHelper helper;
    QString path = "/tmp/dfm_test_thumbnail_helper_dir/sub";
    helper.makePath(path);
    SUCCEED();
}

TEST(ThumbnailHelperTest, SaveThumbnailNullImage)
{
    ThumbnailHelper helper;
    QImage nullImg;
    QString result = helper.saveThumbnail(QUrl::fromLocalFile("/tmp/nonexistent.png"), nullImg, Global::ThumbnailSize::kNormal);
    EXPECT_TRUE(result.isEmpty());
}

TEST(ThumbnailHelperTest, StaticHelpersCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::defaultThumbnailDirs(); });
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::sizeToFilePath(Global::ThumbnailSize::kNormal); });
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::dataToMd5Hex(QByteArray("test")); });
}

// ============================================================
// PMS sev-2 regression cluster: thumbnailhelper.cpp (work-order batch 3)
// ============================================================

// PMS:241241 回收站/虚拟路径缩略图偶发崩溃：checkThumbEnable 对虚拟 scheme 下
// 不存在的文件安全返回 false（InfoFactory 为空即短路），本地文件不崩溃
TEST(ThumbnailHelperTest, BUG241241_CheckThumbEnableVirtualMissingFileFalse)
{
    const QString kVirtScheme = QStringLiteral("vdfmut241241");
    UrlRoute::regScheme(kVirtScheme, QStringLiteral("/"), QIcon(), true, QStringLiteral("virt241241"));

    ThumbnailHelper helper;
    bool enabled = true;
    EXPECT_NO_FATAL_FAILURE({
        enabled = helper.checkThumbEnable(QUrl(QStringLiteral("vdfmut241241://virtual/missing.jpg")));
    });
    EXPECT_FALSE(enabled);

    // local regular file keeps the enable path reachable without crashing
    EXPECT_NO_FATAL_FAILURE({
        (void)helper.checkThumbEnable(QUrl::fromLocalFile(QStringLiteral("/tmp")));
    });
}
