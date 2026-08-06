// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workerpipe.cpp
 * @brief Unit tests for WorkerPipe (libextractor/workerpipe.cpp) — the
 *        dependency-light subset: ctor, dtor, hasPendingPartialMessage (default
 *        false), sendStatus/sendStarted/sendData/sendFailed/sendBatchDone
 *        (all fail gracefully when not initialized — no stdout writes).
 *        initialize() is NOT invoked (it redirects stdin/stdout which would
 *        break the test harness).
 */

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "workerpipe.h"
#include "extractortypes.h"

using namespace dfm_extractor;

TEST(WorkerPipeTest, CtorAndDtorSafe)
{
    {
        WorkerPipe w;
        SUCCEED();
    }
}

TEST(WorkerPipeTest, HasPendingPartialMessageDefaultsFalse)
{
    WorkerPipe w;
    EXPECT_FALSE(w.hasPendingPartialMessage());
}

TEST(WorkerPipeTest, SendStatusFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendStatus(ExtractorStatus::Started, "/tmp/file.txt"));
}

TEST(WorkerPipeTest, SendStartedFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendStarted("/tmp/file.txt"));
}

TEST(WorkerPipeTest, SendDataFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendData("/tmp/file.txt", QByteArray("data")));
}

TEST(WorkerPipeTest, SendFailedFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendFailed("/tmp/file.txt", "error message"));
}

TEST(WorkerPipeTest, SendBatchDoneFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendBatchDone());
}

TEST(WorkerPipeTest, SendStatusBatchDoneFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendStatus(ExtractorStatus::BatchDone));
}

TEST(WorkerPipeTest, SendStatusDataFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendStatus(ExtractorStatus::Data, "/tmp/file.txt", QByteArray("content")));
}

TEST(WorkerPipeTest, SendStatusFailedFailsWhenNotInitialized)
{
    WorkerPipe w;
    EXPECT_FALSE(w.sendStatus(ExtractorStatus::Failed, "/tmp/file.txt", QByteArray("error")));
}
