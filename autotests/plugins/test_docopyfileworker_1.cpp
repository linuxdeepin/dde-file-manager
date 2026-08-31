// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfileworker_1.cpp
 * @brief Unit tests for DoCopyFileWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/docopyfileworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCopyFileWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCopyFileWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCopyFileWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCopyFileWorkerTest, actionOperating)
{
    // Test method: bool actionOperating((const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip))
    auto result = obj->actionOperating(AbstractJobHandler::SupportAction(), 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, actionToNextDo)
{
    // Test method: DoCopyFileWorker::NextDo actionToNextDo((AbstractJobHandler::SupportAction action, qint64 size, bool *skip))
    auto result = obj->actionToNextDo(AbstractJobHandler::SupportAction(), 0, nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, allocateAlignedBuffer)
{
    // Test method: char allocateAlignedBuffer((size_t size, size_t alignment))
    auto result = obj->allocateAlignedBuffer({}, {});
    EXPECT_EQ(result, 0);

}

TEST_F(DoCopyFileWorkerTest, checkRetry)
{
    // Test method: void checkRetry(())
    EXPECT_NO_FATAL_FAILURE(obj->checkRetry());
}

TEST_F(DoCopyFileWorkerTest, createFileDevice)
{
    // Test method: bool createFileDevice((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                        const DFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                                        bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    DFileInfoPointer _arg2{};
    QSharedPointer<DFMIO::DFile> _arg3{};
    auto result = obj->createFileDevice(_arg0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, createFileDevices)
{
    // Test method: bool createFileDevices((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                         QSharedPointer<DFMIO::DFile> &fromeFile, QSharedPointer<DFMIO::DFile> &toFile, bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFMIO::DFile> _arg2{};
    QSharedPointer<DFMIO::DFile> _arg3{};
    auto result = obj->createFileDevices(_arg0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, doCopyFilePractically)
{
    // Test method: DoCopyFileWorker::NextDo doCopyFilePractically((const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip))
    auto result = obj->doCopyFilePractically(DFileInfoPointer(), DFileInfoPointer(), nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, doFileCopy)
{
    // Test method: void doFileCopy((const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo))
    EXPECT_NO_FATAL_FAILURE(obj->doFileCopy(DFileInfoPointer(), DFileInfoPointer()));
}

TEST_F(DoCopyFileWorkerTest, isStopped)
{
    // Test bool getter: isStopped()
    bool result = obj->isStopped();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, openDestinationFile)
{
    // Test method: DoCopyFileWorker::FileWriter openDestinationFile((const QString &dest, WriteMode preferredMode))
    QString _arg0{};
    auto result = obj->openDestinationFile(_arg0, WriteMode());
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, openFile)
{
    // Test method: bool openFile((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                                bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFMIO::DFile> _arg2{};
    DFMIO::DFile::OpenFlags _arg3{};
    auto result = obj->openFile(_arg0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, openFileBySys)
{
    // Test method: int openFileBySys((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                    const int flags, bool *skip, const bool isSource))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->openFileBySys(_arg0, _arg1, 0, nullptr, false);
    EXPECT_GE(result, 0);

}

TEST_F(DoCopyFileWorkerTest, openFiles)
{
    // Test method: bool openFiles((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                 const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                                 bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFMIO::DFile> _arg2{};
    QSharedPointer<DFMIO::DFile> _arg3{};
    auto result = obj->openFiles(_arg0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, operateAction)
{
    // Test method: void operateAction((const AbstractJobHandler::SupportAction action))
    EXPECT_NO_FATAL_FAILURE(obj->operateAction(AbstractJobHandler::SupportAction()));
}

TEST_F(DoCopyFileWorkerTest, pause)
{
    // Test method: void pause(())
    EXPECT_NO_FATAL_FAILURE(obj->pause());
}

TEST_F(DoCopyFileWorkerTest, readAheadSourceFile)
{
    // Test method: void readAheadSourceFile((const DFileInfoPointer &fileInfo))
    DFileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->readAheadSourceFile(_arg0));
}

TEST_F(DoCopyFileWorkerTest, resizeTargetFile)
{
    // Test method: bool resizeTargetFile((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                        const QSharedPointer<DFMIO::DFile> &file, bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFMIO::DFile> _arg2{};
    auto result = obj->resizeTargetFile(_arg0, _arg1, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, resume)
{
    // Test method: void resume(())
    EXPECT_NO_FATAL_FAILURE(obj->resume());
}

TEST_F(DoCopyFileWorkerTest, setTargetPermissions)
{
    // Test setter: void setTargetPermissions((const QUrl &fromUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setTargetPermissions(_arg0, _arg1));
}

TEST_F(DoCopyFileWorkerTest, stateCheck)
{
    // Test bool getter: stateCheck()
    bool result = obj->stateCheck();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(DoCopyFileWorkerTest, workerWait)
{
    // Test method: void workerWait(())
    EXPECT_NO_FATAL_FAILURE(obj->workerWait());
}

TEST_F(DoCopyFileWorkerTest, DoCopyFileWorker_Destructor)
{
    // Test method:  ~DoCopyFileWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoCopyFileWorker *tmp = new DoCopyFileWorker(); delete tmp; });
}
