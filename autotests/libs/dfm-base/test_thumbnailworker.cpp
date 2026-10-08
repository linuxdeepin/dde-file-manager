// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailworker.cpp
 * @brief Unit tests for ThumbnailWorker (thumbnailworker.cpp)
 *
 * ThumbnailWorker is a QObject with a public constructor and a
 * registerCreator() boolean contract (new MIME succeeds, duplicate is
 * rejected). The worker thread is not started by registerCreator, so no
 * real thumbnail generation runs. stop() is also exercised.
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/thumbnail/thumbnailworker.h>

#include <QString>
#include <QImage>
#include <QSignalSpy>
#include <QUrl>
#include <QFile>

using namespace dfmbase;

namespace {
ThumbnailWorker::ThumbnailCreator utNoopCreator =
    [](const QString &, DFMGLOBAL_NAMESPACE::ThumbnailSize) {
        return QImage();
    };
}   // namespace

TEST(ThumbnailWorkerTest, ConstructAndDestructWithoutCrash)
{
    {
        ThumbnailWorker worker;
        (void)worker;
    }
    SUCCEED();
}

TEST(ThumbnailWorkerTest, RegisterNewCreatorSucceeds)
{
    ThumbnailWorker worker;
    bool ok = worker.registerCreator(QStringLiteral("application/x-ut-tw-test"),
                                      utNoopCreator);
    EXPECT_TRUE(ok);
}

TEST(ThumbnailWorkerTest, RegisterDuplicateCreatorReturnsFalse)
{
    ThumbnailWorker worker;
    const QString mime = QStringLiteral("application/x-ut-tw-dup");
    ASSERT_TRUE(worker.registerCreator(mime, utNoopCreator));
    EXPECT_FALSE(worker.registerCreator(mime, utNoopCreator));
}

TEST(ThumbnailWorkerTest, StopIsCallableAndDoesNotCrash)
{
    ThumbnailWorker worker;
    worker.stop();
    SUCCEED();   // reached only if stop did not crash
}

// ============================================================
// PMS sev-2 regression cluster: thumbnailworker.cpp (work-order batch 3)
// ============================================================

// PMS:184859 挂载 smb 关闭鉴权窗口双击面包屑崩溃：createThumbnail 对不存在/
// 无法生成的文件必须同步回退到 thumbnailCreateFailed，不再空指针崩溃
TEST(ThumbnailWorkerTest, BUG184859_CreateThumbnailUn_generatableEmitsFailed)
{
    ThumbnailWorker worker;
    QSignalSpy failedSpy(&worker, &ThumbnailWorker::thumbnailCreateFailed);
    QSignalSpy okSpy(&worker, &ThumbnailWorker::thumbnailCreateFinished);
    const QUrl url = QUrl::fromLocalFile(QStringLiteral("/tmp/dfm_ut_thumb_184859_missing.txt"));
    QFile::remove(url.toLocalFile());

    EXPECT_NO_FATAL_FAILURE({ worker.createThumbnail(url, Global::ThumbnailSize::kNormal); });

    EXPECT_EQ(okSpy.count(), 0);
    ASSERT_EQ(failedSpy.count(), 1);
    // emitted payload is d->originalUrl (the task url); outside a task
    // context it is empty — the regression contract is "failed, no crash"
    EXPECT_EQ(failedSpy.at(0).at(0).value<QUrl>(), QUrl());
}
