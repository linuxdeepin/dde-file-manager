// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractworker.cpp
 * @brief Unit tests for AbstractWorker Mid-priority methods (dfmplugin-fileoperations)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "fileoperations/fileoperationutils/abstractworker.h"

using namespace dfmplugin_fileoperations;

class AbstractWorkerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(AbstractWorkerTest, checkRetry)
{
    // Instance method checkRetry
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.checkRetry(); });
}

TEST_F(AbstractWorkerTest, doOperateWork)
{
    // Instance method doOperateWork
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.doOperateWork(AbstractJobHandler::SupportActions(), AbstractJobHandler::JobErrorType(), 0); });
}

TEST_F(AbstractWorkerTest, emitProgressChangedNotify)
{
    // Instance method emitProgressChangedNotify
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.emitProgressChangedNotify(0); });
}

TEST_F(AbstractWorkerTest, endWork)
{
    // Instance method endWork
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.endWork(); });
}

TEST_F(AbstractWorkerTest, isStopped)
{
    // Instance method isStopped
    AbstractWorker obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isStopped(); });
    (void)result;
}

TEST_F(AbstractWorkerTest, parentUrl)
{
    // Instance method parentUrl
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.parentUrl(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(AbstractWorkerTest, pause)
{
    // Instance method pause
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.pause(); });
}

TEST_F(AbstractWorkerTest, resumeThread)
{
    // Instance method resumeThread
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.resumeThread({}); });
}

TEST_F(AbstractWorkerTest, setStat)
{
    // Instance method setStat
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setStat(AbstractJobHandler::JobState()); });
}

TEST_F(AbstractWorkerTest, stateCheck)
{
    // Instance method stateCheck
    AbstractWorker obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.stateCheck(); });
    (void)result;
}

TEST_F(AbstractWorkerTest, stop)
{
    // Instance method stop
    AbstractWorker obj;
    EXPECT_NO_FATAL_FAILURE({ obj.stop(); });
}

TEST_F(AbstractWorkerTest, AbstractWorker)
{
    // AbstractWorker
    SUCCEED();
}
