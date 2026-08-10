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
