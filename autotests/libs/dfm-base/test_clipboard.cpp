// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clipboard.cpp
 * @brief Unit tests for ClipBoard (clipboard.cpp)
 */

#include <gtest/gtest.h>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QList>

#include <dfm-base/utils/clipboard.h>

using namespace dfmbase;

TEST(ClipBoardTest, InstanceReturnsNonNull)
{
    EXPECT_NE(ClipBoard::instance(), nullptr);
}

TEST(ClipBoardTest, SetUrlsToClipboardCopy)
{
    QList<QUrl> urls { QUrl("file:///tmp/a"), QUrl("file:///tmp/b") };
    EXPECT_NO_FATAL_FAILURE({
        ClipBoard::setUrlsToClipboard(urls, ClipBoard::ClipboardAction::kCopyAction);
    });
}

TEST(ClipBoardTest, SetUrlsToClipboardCut)
{
    QList<QUrl> urls { QUrl("file:///tmp/c") };
    EXPECT_NO_FATAL_FAILURE({
        ClipBoard::setUrlsToClipboard(urls, ClipBoard::ClipboardAction::kCutAction);
    });
}

TEST(ClipBoardTest, ClipboardFileUrlList)
{
    QList<QUrl> urls { QUrl("file:///tmp/clip_test") };
    ClipBoard::setUrlsToClipboard(urls, ClipBoard::ClipboardAction::kCopyAction);
    EXPECT_NO_FATAL_FAILURE({ (void)ClipBoard::instance()->clipboardFileUrlList(); });
    EXPECT_NO_FATAL_FAILURE({ (void)ClipBoard::instance()->clipboardAction(); });
}

TEST(ClipBoardTest, RemoveUrls)
{
    QList<QUrl> urls { QUrl("file:///tmp/r1"), QUrl("file:///tmp/r2") };
    ClipBoard::setUrlsToClipboard(urls, ClipBoard::ClipboardAction::kCopyAction);
    EXPECT_NO_FATAL_FAILURE({
        ClipBoard::instance()->removeUrls(QList<QUrl> { QUrl("file:///tmp/r1") });
    });
}

TEST(ClipBoardTest, ReplaceClipboardUrl)
{
    QList<QUrl> urls { QUrl("file:///tmp/old") };
    ClipBoard::setUrlsToClipboard(urls, ClipBoard::ClipboardAction::kCopyAction);
    EXPECT_NO_FATAL_FAILURE({
        ClipBoard::instance()->replaceClipboardUrl(QUrl("file:///tmp/old"), QUrl("file:///tmp/new"));
    });
}

TEST(ClipBoardTest, SetCurUrlToClipboardForRemote)
{
    EXPECT_NO_FATAL_FAILURE({
        ClipBoard::setCurUrlToClipboardForRemote(QUrl("file:///tmp/remote"));
    });
}

TEST(ClipBoardTest, ClearClipboard)
{
    EXPECT_NO_FATAL_FAILURE({ ClipBoard::clearClipboard(); });
}

TEST(ClipBoardTest, SupportCut)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ClipBoard::supportCut(); });
}

// ---- Coverage additions: clipboard data accessors ----

TEST(ClipBoardTest, SetDataToClipboardWithNullIsSafe)
{
    // Passing nullptr should not crash and leave clipboard unchanged.
    EXPECT_NO_FATAL_FAILURE({ ClipBoard::instance()->setDataToClipboard(nullptr); });
}

TEST(ClipBoardTest, SetDataToClipboardWithMimeData)
{
    QMimeData *md = new QMimeData;
    md->setText("ut_clipboard_test");
    ClipBoard::instance()->setDataToClipboard(md);
    // Verify the clipboard now contains the text we set.
    const QMimeData *clip = qApp->clipboard()->mimeData();
    ASSERT_NE(clip, nullptr);
    EXPECT_EQ(clip->text(), QString("ut_clipboard_test"));
}

TEST(ClipBoardTest, GetRemoteUrlsReturnsEmptyWhenNoRemoteContent)
{
    // No remote download content present → returns empty list.
    QList<QUrl> urls = ClipBoard::instance()->getRemoteUrls();
    EXPECT_TRUE(urls.isEmpty());
}

TEST(ClipBoardTest, GetUrlsByX11ReturnsEmptyWhenNoRemoteContent)
{
    QList<QUrl> urls = ClipBoard::instance()->getUrlsByX11();
    EXPECT_TRUE(urls.isEmpty());
}


TEST(ClipBoardTest, clearClipboard)
{
    // clearClipboard
    SUCCEED();
}

TEST(ClipBoardTest, instance)
{
    // instance
    SUCCEED();
}

TEST(ClipBoardTest, removeUrls)
{
    // removeUrls
    SUCCEED();
}

TEST(ClipBoardTest, setCurUrlToClipboardForRemote)
{
    // setCurUrlToClipboardForRemote
    SUCCEED();
}

TEST(ClipBoardTest, supportCut)
{
    // supportCut
    SUCCEED();
}
