// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QSharedPointer>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventhandler.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-framework/dpf.h>

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class TestFileOperationsEventHandler : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        handler = FileOperationsEventHandler::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    /*!\
     * \brief Create a mock job info pointer for testing
     */
    JobInfoPointer createMockJobInfo(AbstractJobHandler::JobType jobType,
                                     const QList<QUrl> &srcUrls = {},
                                     const QList<QUrl> &destUrls = {},
                                     bool hasError = false)
    {
        JobInfoPointer jobInfo(new QMap<quint8, QVariant>());

        jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
        jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(srcUrls));
        jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey, QVariant::fromValue(destUrls));

        if (hasError) {
            jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                            QVariant::fromValue(AbstractJobHandler::JobErrorType::kProrogramError));
            jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey, "Test error message");
        } else {
            jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                            QVariant::fromValue(AbstractJobHandler::JobErrorType::kNoError));
        }

        return jobInfo;
    }

    stub_ext::StubExt stub;
    FileOperationsEventHandler *handler { nullptr };
};

// ========== FileOperationsEventHandler::instance() Tests ==========

TEST_F(TestFileOperationsEventHandler, Instance_ReturnsSingleton)
{
    FileOperationsEventHandler *instance1 = FileOperationsEventHandler::instance();
    FileOperationsEventHandler *instance2 = FileOperationsEventHandler::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

// ========== FileOperationsEventHandler::publishJobResultEvent() Tests ==========

TEST_F(TestFileOperationsEventHandler, PublishJobResultEvent_CopyType)
{
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/src1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/dest1.txt") };

    handler->publishJobResultEvent(AbstractJobHandler::JobType::kCopyType, srcUrls, destUrls, {}, true, "");

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, PublishJobResultEvent_CutType)
{
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/src1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/dest1.txt") };

    handler->publishJobResultEvent(AbstractJobHandler::JobType::kCutType, srcUrls, destUrls, {}, true, "");

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, PublishJobResultEvent_DeleteType)
{

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/src1.txt") };

    handler->publishJobResultEvent(AbstractJobHandler::JobType::kDeleteType, srcUrls, {}, {}, true, "");

    SUCCEED();
}

// ========== FileOperationsEventHandler::removeUrlsInClipboard() Tests ==========

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_DeleteType)
{
    bool removeUrlsCalled = false;
    QList<QUrl> removedUrls;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled, &removedUrls](ClipBoard *, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
        removedUrls = urls;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kDeleteType, srcUrls, {}, true);

    EXPECT_TRUE(removeUrlsCalled);
    EXPECT_EQ(removedUrls, srcUrls);
}

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_MoveToTrashType)
{
    bool removeUrlsCalled = false;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled](ClipBoard *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kMoveToTrashType, srcUrls, {}, true);

    EXPECT_TRUE(removeUrlsCalled);
}

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_CutType)
{
    bool removeUrlsCalled = false;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled](ClipBoard *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kCutType, srcUrls, {}, true);

    EXPECT_TRUE(removeUrlsCalled);
}

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_CleanTrashType)
{
    bool removeUrlsCalled = false;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled](ClipBoard *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
    });

    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/.Trash/file1.txt") };

    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kCleanTrashType, {}, destUrls, true);

    EXPECT_TRUE(removeUrlsCalled);
}

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_CopyTypeDoesNotRemove)
{
    bool removeUrlsCalled = false;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled](ClipBoard *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kCopyType, srcUrls, {}, true);

    EXPECT_FALSE(removeUrlsCalled);
}

TEST_F(TestFileOperationsEventHandler, RemoveUrlsInClipboard_FailedJobDoesNotRemove)
{
    bool removeUrlsCalled = false;

    stub.set_lamda(&ClipBoard::removeUrls, [&removeUrlsCalled](ClipBoard *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        removeUrlsCalled = true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    // Pass false for ok parameter
    handler->removeUrlsInClipboard(AbstractJobHandler::JobType::kDeleteType, srcUrls, {}, false);

    EXPECT_FALSE(removeUrlsCalled);
}

// ========== FileOperationsEventHandler::handleJobResult() Tests ==========

TEST_F(TestFileOperationsEventHandler, HandleJobResult_NullPointer)
{
    // Should not crash with null pointer
    handler->handleJobResult(AbstractJobHandler::JobType::kCopyType, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, HandleJobResult_UnknownJobType)
{
    // Create a mock job handle
    JobHandlePointer jobHandle(new AbstractJobHandler());

    // Should not crash with unknown job type
    handler->handleJobResult(AbstractJobHandler::JobType::kUnknow, jobHandle);

    SUCCEED();
}

// ========== FileOperationsEventHandler::handleErrorNotify() Tests ==========

TEST_F(TestFileOperationsEventHandler, HandleErrorNotify_NullJobInfo)
{
    // Should not crash with null job info
    handler->handleErrorNotify(nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, HandleErrorNotify_WithError)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                    QVariant::fromValue(AbstractJobHandler::JobErrorType::kProrogramError));
    jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey, "Test error");

    // Should process error without crashing
    handler->handleErrorNotify(jobInfo);

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, HandleErrorNotify_NoError)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    jobInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                    QVariant::fromValue(AbstractJobHandler::JobErrorType::kNoError));

    // Should process without error
    handler->handleErrorNotify(jobInfo);

    SUCCEED();
}

// ========== FileOperationsEventHandler::handleFinishedNotify() Tests ==========

TEST_F(TestFileOperationsEventHandler, HandleFinishedNotify_NullJobInfo)
{
    // Should not crash with null job info
    handler->handleFinishedNotify(nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventHandler, HandleFinishedNotify_MissingJobTypeKey)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());

    // Should handle missing job type key gracefully
    handler->handleFinishedNotify(jobInfo);

    SUCCEED();
}
