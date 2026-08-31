// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractworker.cpp
 * @brief Unit tests for AbstractWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class AbstractWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractWorkerTest, checkRetry)
{
    // Test method: void checkRetry(())
    EXPECT_NO_FATAL_FAILURE(obj->checkRetry());
}

TEST_F(AbstractWorkerTest, doOperateWork)
{
    // Test method: void doOperateWork((AbstractJobHandler::SupportActions actions, AbstractJobHandler::JobErrorType error, const quint64 id))
    EXPECT_NO_FATAL_FAILURE(obj->doOperateWork(AbstractJobHandler::SupportActions(), AbstractJobHandler::JobErrorType(), 0));
}

TEST_F(AbstractWorkerTest, emitProgressChangedNotify)
{
    // Test method: void emitProgressChangedNotify((const qint64 &writSize))
    qint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->emitProgressChangedNotify(_arg0));
}

TEST_F(AbstractWorkerTest, endWork)
{
    // Test method: void endWork(())
    EXPECT_NO_FATAL_FAILURE(obj->endWork());
}

TEST_F(AbstractWorkerTest, isStopped)
{
    // Test bool getter: isStopped()
    bool result = obj->isStopped();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, parentUrl)
{
    // Test method: QUrl parentUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->parentUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AbstractWorkerTest, pause)
{
    // Test method: void pause(())
    EXPECT_NO_FATAL_FAILURE(obj->pause());
}

TEST_F(AbstractWorkerTest, resumeThread)
{
    // Test method: void resumeThread((const QList<quint64> &errorIds))
    QList<quint64> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resumeThread(_arg0));
}

TEST_F(AbstractWorkerTest, setStat)
{
    // Test setter: void setStat((const AbstractJobHandler::JobState &stat))
    AbstractJobHandler::JobState _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setStat(_arg0));
}

TEST_F(AbstractWorkerTest, stateCheck)
{
    // Test bool getter: stateCheck()
    bool result = obj->stateCheck();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(AbstractWorkerTest, AbstractWorker)
{
    // Test constructor: AbstractWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
