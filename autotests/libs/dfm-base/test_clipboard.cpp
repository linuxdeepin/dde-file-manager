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
    EXPECT_NO_FATAL_FAILURE({ ClipBoard::instance()->setDataToClipboard(nullptr); });
}

TEST(ClipBoardTest, SetDataToClipboardWithMimeData)
{
    QMimeData *md = new QMimeData;
    md->setText("ut_clipboard_test");
    EXPECT_NO_FATAL_FAILURE({ ClipBoard::instance()->setDataToClipboard(md); });
}

TEST(ClipBoardTest, GetRemoteUrlsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ClipBoard::instance()->getRemoteUrls(); });
}

TEST(ClipBoardTest, GetUrlsByX11Callable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ClipBoard::instance()->getUrlsByX11(); });
}
