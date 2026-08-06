// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_controllerpipe.cpp
 * @brief Unit tests for ControllerPipe (libextractor/controllerpipe.cpp) — the
 *        dependency-light subset: ctor, dtor, isRunning (false by default),
 *        processId (-1 by default), hasPendingPartialMessage (false), start
 *        with empty path (early return), extractBatch when not running
 *        (early return). The actual subprocess spawning is not exercised.
 */

#include <gtest/gtest.h>
#include <QVector>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "controllerpipe.h"

using namespace dfm_extractor;

TEST(ControllerPipeTest, CtorAndDtorSafe)
{
    {
        ControllerPipe c;
        SUCCEED();
    }
}

TEST(ControllerPipeTest, IsRunningFalseByDefault)
{
    ControllerPipe c;
    EXPECT_FALSE(c.isRunning());
}

TEST(ControllerPipeTest, ProcessIdNegativeByDefault)
{
    ControllerPipe c;
    EXPECT_EQ(c.processId(), -1);
}

TEST(ControllerPipeTest, HasPendingPartialMessageFalseByDefault)
{
    ControllerPipe c;
    EXPECT_FALSE(c.hasPendingPartialMessage());
}

TEST(ControllerPipeTest, StartEmptyPathReturnsFalse)
{
    ControllerPipe c;
    EXPECT_FALSE(c.start(QString()));
}

TEST(ControllerPipeTest, StartNonExistentPathReturnsFalse)
{
    ControllerPipe c;
    EXPECT_FALSE(c.start("/nonexistent/extractor/path"));
}

TEST(ControllerPipeTest, ExtractBatchFailsWhenNotRunning)
{
    ControllerPipe c;
    QVector<QString> files { "/tmp/a.txt" };
    EXPECT_FALSE(c.extractBatch(files));
}

TEST(ControllerPipeTest, ExtractBatchEmptyFilesFailsWhenNotRunning)
{
    ControllerPipe c;
    QVector<QString> empty;
    EXPECT_FALSE(c.extractBatch(empty));
}

TEST(ControllerPipeTest, StopWhenNotRunningIsSafe)
{
    ControllerPipe c;
    EXPECT_NO_FATAL_FAILURE({ c.stop(); });
}

TEST(ControllerPipeTest, MultipleStopCallsSafe)
{
    ControllerPipe c;
    c.stop();
    c.stop();
    SUCCEED();
}
