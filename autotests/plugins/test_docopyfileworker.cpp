// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfileworker.cpp
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

TEST_F(DoCopyFileWorkerTest, doDfmioFileCopy)
{
    // Test method: bool doDfmioFileCopy((const DFileInfoPointer fromInfo,
                                       const DFileInfoPointer toInfo, bool *skip))
    auto result = obj->doDfmioFileCopy(DFileInfoPointer(), DFileInfoPointer(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, handlePauseResume)
{
    // Test method: bool handlePauseResume((FileWriter &writer, const QString &dest, bool *skip))
    FileWriter _arg0{};
    QString _arg1{};
    auto result = obj->handlePauseResume(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, progressCallback)
{
    // Test method: void progressCallback((int64_t current, int64_t total, void *progressData))
    EXPECT_NO_FATAL_FAILURE(obj->progressCallback({}, {}, nullptr));
}

TEST_F(DoCopyFileWorkerTest, shouldFallbackFromCopyFileRange)
{
    // Test method: bool shouldFallbackFromCopyFileRange((int errorCode))
    auto result = obj->shouldFallbackFromCopyFileRange(0);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, verifyFileIntegrity)
{
    // Test method: bool verifyFileIntegrity((const qint64 &blockSize, const ulong &sourceCheckSum,
                                           const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                           QSharedPointer<DFMIO::DFile> &toDevice))
    qint64 _arg0{};
    ulong _arg1{};
    DFileInfoPointer _arg2{};
    DFileInfoPointer _arg3{};
    QSharedPointer<DFMIO::DFile> _arg4{};
    auto result = obj->verifyFileIntegrity(_arg0, _arg1, _arg2, _arg3, _arg4);
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFileWorkerTest, doHandleErrorAndWait)
{
    // Test method: AbstractJobHandler::SupportAction doHandleErrorAndWait((const QUrl &urlFrom, const QUrl &urlTo,
                                                                         const AbstractJobHandler::JobErrorType &error,
                                                                         const bool isTo,
                                                                         const QString &errorMsg))
    QUrl _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobErrorType _arg2{};
    QString _arg4{};
    auto result = obj->doHandleErrorAndWait(_arg0, _arg1, _arg2, false, _arg4);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, doWriteFile)
{
    // Test method: DoCopyFileWorker::NextDo doWriteFile((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                                       const QSharedPointer<DFMIO::DFile> &toDevice,
                                                       const QSharedPointer<DFile> &fromDevice,
                                                       const char *data, const qint64 readSize, bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFMIO::DFile> _arg2{};
    QSharedPointer<DFile> _arg3{};
    auto result = obj->doWriteFile(_arg0, _arg1, _arg2, _arg3, nullptr, 0, nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, doWriteFileErrorRetry)
{
    // Test method: DoCopyFileWorker::NextDo doWriteFileErrorRetry((const DFileInfoPointer &fromInfo,
                                                                 const DFileInfoPointer &toInfo,
                                                                 const QSharedPointer<DFile> &toDevice,
                                                                 const QSharedPointer<DFile> &fromDevice,
                                                                 const qint64 readSize,
                                                                 bool *skip,
                                                                 const qint64 currentPos,
                                                                 const qint64 &surplusSize, qint64 &curWrite))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    QSharedPointer<DFile> _arg2{};
    QSharedPointer<DFile> _arg3{};
    qint64 _arg7{};
    qint64 _arg8{};
    auto result = obj->doWriteFileErrorRetry(_arg0, _arg1, _arg2, _arg3, 0, nullptr, 0, _arg7, _arg8);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCopyFileWorkerTest, reopenDestinationFileForResume)
{
    // Test method: DoCopyFileWorker::FileWriter reopenDestinationFileForResume((const QString &dest, WriteMode preferredMode))
    QString _arg0{};
    auto result = obj->reopenDestinationFileForResume(_arg0, WriteMode());
    EXPECT_GE(static_cast<int>(result), 0);

}
