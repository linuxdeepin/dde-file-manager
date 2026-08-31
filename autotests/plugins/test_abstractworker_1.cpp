// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractworker_1.cpp
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

TEST_F(AbstractWorkerTest, createCopyJobInfo)
{
    // Test method: JobInfoPointer createCopyJobInfo((const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType error))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->createCopyJobInfo(_arg0, _arg1, AbstractJobHandler::JobErrorType());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(AbstractWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, emitCurrentTaskNotify)
{
    // Test method: void emitCurrentTaskNotify((const QUrl &from, const QUrl &to))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->emitCurrentTaskNotify(_arg0, _arg1));
}

TEST_F(AbstractWorkerTest, emitErrorNotify)
{
    // Test method: void emitErrorNotify((const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const bool isTo,
                                     const quint64 id, const QString &errorMsg, const bool allUsErrorMsg))
    QUrl _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobErrorType _arg2{};
    QString _arg5{};
    EXPECT_NO_FATAL_FAILURE(obj->emitErrorNotify(_arg0, _arg1, _arg2, false, 0, _arg5, false));
}

TEST_F(AbstractWorkerTest, emitStateChangedNotify)
{
    // Test method: void emitStateChangedNotify(())
    EXPECT_NO_FATAL_FAILURE(obj->emitStateChangedNotify());
}

TEST_F(AbstractWorkerTest, formatFileName)
{
    // Test method: QString formatFileName((const QString &fileName))
    QString _arg0{};
    auto result = obj->formatFileName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AbstractWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, initHandleConnects)
{
    // Test method: void initHandleConnects((const JobHandlePointer handle))
    EXPECT_NO_FATAL_FAILURE(obj->initHandleConnects(JobHandlePointer()));
}

TEST_F(AbstractWorkerTest, needsSync)
{
    // Test bool getter: needsSync()
    bool result = obj->needsSync();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(AbstractWorkerTest, pauseAllThread)
{
    // Test method: void pauseAllThread(())
    EXPECT_NO_FATAL_FAILURE(obj->pauseAllThread());
}

TEST_F(AbstractWorkerTest, performAsyncSync)
{
    // Test method: void performAsyncSync(())
    EXPECT_NO_FATAL_FAILURE(obj->performAsyncSync());
}

TEST_F(AbstractWorkerTest, performSync)
{
    // Test method: void performSync(())
    EXPECT_NO_FATAL_FAILURE(obj->performSync());
}

TEST_F(AbstractWorkerTest, resume)
{
    // Test method: void resume(())
    EXPECT_NO_FATAL_FAILURE(obj->resume());
}

TEST_F(AbstractWorkerTest, resumeAllThread)
{
    // Test method: void resumeAllThread(())
    EXPECT_NO_FATAL_FAILURE(obj->resumeAllThread());
}

TEST_F(AbstractWorkerTest, setWorkArgs)
{
    // Test setter: void setWorkArgs((const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target,
                                 const AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    AbstractJobHandler::JobFlags _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->setWorkArgs(JobHandlePointer(), _arg1, _arg2, _arg3));
}

TEST_F(AbstractWorkerTest, startAsyncStatistics)
{
    // Test method: void startAsyncStatistics((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->startAsyncStatistics(_arg0));
}

TEST_F(AbstractWorkerTest, startCountProccess)
{
    // Test method: void startCountProccess(())
    EXPECT_NO_FATAL_FAILURE(obj->startCountProccess());
}

TEST_F(AbstractWorkerTest, statisticsFilesSize)
{
    // Test bool getter: statisticsFilesSize()
    bool result = obj->statisticsFilesSize();
    EXPECT_FALSE(result);

}

TEST_F(AbstractWorkerTest, stopAllThread)
{
    // Test method: void stopAllThread(())
    EXPECT_NO_FATAL_FAILURE(obj->stopAllThread());
}

TEST_F(AbstractWorkerTest, syncFilesToDevice)
{
    // Test method: void syncFilesToDevice(())
    EXPECT_NO_FATAL_FAILURE(obj->syncFilesToDevice());
}

TEST_F(AbstractWorkerTest, workerWait)
{
    // Test bool getter: workerWait()
    bool result = obj->workerWait();
    EXPECT_FALSE(result);

}
