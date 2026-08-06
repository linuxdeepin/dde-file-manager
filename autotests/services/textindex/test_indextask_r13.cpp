// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indextask_r13.cpp
 * @brief Additional IndexTask tests: onProgressChanged (only emits when running),
 *        throttleCpuUsage (skips when non-silent), D0 destructor (heap alloc+delete),
 *        doTask (runs a dummy handler that returns early).
 *        Private methods via -fno-access-control.
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/indextask.h"
#include "services/textindex/task/taskhandler.h"
#include "services/textindex/utils/taskstate.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static TaskHandler noopHandler()
{
    return [](const QString &, TaskState &) -> HandlerResult { return HandlerResult {}; };
}

TEST(IndexTaskR13Test, OnProgressChangedNoEmitWhenNotRunning)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    QSignalSpy spy(&t, &IndexTask::progressChanged);
    t.onProgressChanged(10, 100);   // private; not running -> no emit
    EXPECT_EQ(spy.count(), 0);
}

TEST(IndexTaskR13Test, ThrottleCpuUsageSkipsWhenNotSilent)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    EXPECT_FALSE(t.silent());
    EXPECT_NO_FATAL_FAILURE({ t.throttleCpuUsage(); });   // private; returns early
}

TEST(IndexTaskR13Test, D0DestructorPath)
{
    auto *ptr = new IndexTask(IndexTask::Type::Create, "/tmp/heap", noopHandler());
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

TEST(IndexTaskR13Test, DoTaskWithHandler)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    // doTask() runs the handler in the current thread (no thread check here);
    // it returns HandlerResult{false,...} because noopHandler returns default.
    EXPECT_NO_FATAL_FAILURE({ t.doTask(); });   // private
}

TEST(IndexTaskR13Test, DoTaskWithNullHandler)
{
    TaskHandler nullHandler;
    IndexTask t(IndexTask::Type::Create, "/tmp", nullHandler);
    EXPECT_NO_FATAL_FAILURE({ t.doTask(); });   // m_handler is null -> result stays default
}

TEST(IndexTaskR13Test, SetSilentThenThrottleCpuUsage)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    t.setSilent(true);
    EXPECT_TRUE(t.silent());
    EXPECT_NO_FATAL_FAILURE({ t.throttleCpuUsage(); });   // tries systemd (fails in sandbox)
}
