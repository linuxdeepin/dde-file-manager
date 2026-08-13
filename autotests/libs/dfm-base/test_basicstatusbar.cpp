// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicstatusbar.cpp
 * @brief Unit tests for BasicStatusBar (basicstatusbar.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QSize>
#include <QString>
#include <QUrl>
#include <QLabel>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <mutex>

using namespace dfmbase;

TEST(BasicStatusBarTest, ConstructDoesNotCrash)
{
    BasicStatusBar bar;
    (void)bar;
}

TEST(BasicStatusBarTest, SizeHintHeightAtLeast32)
{
    BasicStatusBar bar;
    QSize hint = bar.sizeHint();
    EXPECT_GE(hint.height(), 32);
}

TEST(BasicStatusBarTest, ClearLayoutAndAnchorsDoesNotCrash)
{
    BasicStatusBar bar;
    bar.clearLayoutAndAnchors();
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemSelectedWithNoItemsDoesNotCrash)
{
    BasicStatusBar bar;
    bar.itemSelected(0, 0, 0, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, SetTipTextDoesNotCrash)
{
    BasicStatusBar bar;
    bar.setTipText(QStringLiteral("test tip"));
    SUCCEED();
}

// ============================================================
// Additional coverage for BasicStatusBar
// ============================================================

TEST(BasicStatusBarTest, ItemSelectedWithFilesOnly)
{
    BasicStatusBar bar;
    bar.itemSelected(3, 0, 1024, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemSelectedWithFoldersOnly)
{
    BasicStatusBar bar;
    bar.itemSelected(0, 2, 0, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemSelectedWithBoth)
{
    BasicStatusBar bar;
    QList<QUrl> folderUrls = {QUrl::fromLocalFile("/tmp")};
    bar.itemSelected(5, 2, 2048, folderUrls);
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemSelectedLargeSize)
{
    BasicStatusBar bar;
    bar.itemSelected(1, 0, 1024LL * 1024 * 1024 * 5, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemCounted)
{
    BasicStatusBar bar;
    bar.itemCounted(42);
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemCountedZero)
{
    BasicStatusBar bar;
    bar.itemCounted(0);
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemCountedNegative)
{
    BasicStatusBar bar;
    bar.itemCounted(-1);
    SUCCEED();
}

TEST(BasicStatusBarTest, ClearLayoutAndAnchorsTwice)
{
    BasicStatusBar bar;
    bar.clearLayoutAndAnchors();
    bar.clearLayoutAndAnchors();
    SUCCEED();
}

TEST(BasicStatusBarTest, AddWidget)
{
    BasicStatusBar bar;
    QLabel label("test");
    bar.addWidget(&label);
    SUCCEED();
}

TEST(BasicStatusBarTest, InsertWidget)
{
    BasicStatusBar bar;
    QLabel label("inserted");
    bar.insertWidget(0, &label);
    SUCCEED();
}

TEST(BasicStatusBarTest, AddWidgetWithStretch)
{
    BasicStatusBar bar;
    QLabel label("stretch");
    bar.addWidget(&label, 1, Qt::AlignRight);
    SUCCEED();
}

TEST(BasicStatusBarTest, InsertWidgetWithStretch)
{
    BasicStatusBar bar;
    QLabel label("insertStretch");
    bar.insertWidget(0, &label, 1, Qt::AlignLeft);
    SUCCEED();
}

TEST(BasicStatusBarTest, SetTipTextMultipleTimes)
{
    BasicStatusBar bar;
    bar.setTipText("first");
    bar.setTipText("second");
    bar.setTipText("");
    SUCCEED();
}

TEST(BasicStatusBarTest, UpdateStatusMessageAfterClear)
{
    BasicStatusBar bar;
    bar.clearLayoutAndAnchors();
    // After clear, tip is gone, so updateStatusMessage should handle null
    bar.itemSelected(1, 0, 100, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, SizeHintAfterClear)
{
    BasicStatusBar bar;
    bar.clearLayoutAndAnchors();
    QSize hint = bar.sizeHint();
    EXPECT_GE(hint.height(), 32);
}

TEST(BasicStatusBarTest, ItemSelectedWithFileInfoList)
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/testfile.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);

    BasicStatusBar bar;
    QList<FileInfo *> infoList { info.data() };
    EXPECT_NO_FATAL_FAILURE({
        bar.itemSelected(infoList);
    });
}

TEST(BasicStatusBarTest, ItemSelectedWithEmptyFileInfoList)
{
    BasicStatusBar bar;
    QList<FileInfo *> emptyList;
    EXPECT_NO_FATAL_FAILURE({
        bar.itemSelected(emptyList);
    });
}

TEST(BasicStatusBarTest, ItemSelectedWithDirFileInfo)
{
    static std::once_flag flag2;
    std::call_once(flag2, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(tmpDir.path()));
    ASSERT_NE(info, nullptr);

    BasicStatusBar bar;
    QList<FileInfo *> infoList { info.data() };
    EXPECT_NO_FATAL_FAILURE({
        bar.itemSelected(infoList);
    });
}
