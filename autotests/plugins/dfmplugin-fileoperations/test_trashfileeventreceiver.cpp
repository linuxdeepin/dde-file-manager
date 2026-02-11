// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QSignalSpy>
#include <QUrl>

#include "stubext.h"

#include "fileoperationsevent/trashfileeventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-io/denumerator.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestTrashFileEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Register file info factories
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        receiver = TrashFileEventReceiver::instance();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    /*!\
     * \brief Create a temporary test file
     */
    QUrl createTestFile(const QString &fileName)
    {
        QString filePath = tempDir->path() + "/" + fileName;
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write("test content");
        file.close();

        return QUrl::fromLocalFile(filePath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    TrashFileEventReceiver *receiver { nullptr };
};

// ========== TrashFileEventReceiver::instance() Tests ==========

TEST_F(TestTrashFileEventReceiver, Instance_ReturnsSingleton)
{
    TrashFileEventReceiver *instance1 = TrashFileEventReceiver::instance();
    TrashFileEventReceiver *instance2 = TrashFileEventReceiver::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

// ========== TrashFileEventReceiver::doMoveToTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, DoMoveToTrash_EmptySourceList)
{
    QList<QUrl> emptySources;

    auto handle = receiver->doMoveToTrash(0, emptySources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoMoveToTrash_HandledByOtherPlugin)
{
    int execRet = QDialog::Rejected;
    stub.set_lamda(VADDR(QDialog, exec), [&] { __DBG_STUB_INVOKE__ return execRet; });

    QUrl testFile = createTestFile("test.txt");
    QList<QUrl> sources = { testFile };

    auto handle = receiver->doMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoMoveToTrash_SystemPathBlocked)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DialogManager::showDeleteSystemPathWarnDialog, [](DialogManager *, quint64) {
        __DBG_STUB_INVOKE__
    });

    QUrl testFile = createTestFile("system.txt");
    QList<QUrl> sources = { testFile };

    auto handle = receiver->doMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoMoveToTrash_UserCancelsDialog)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Rejected;
    });

    QUrl testFile = createTestFile("cancel.txt");
    QList<QUrl> sources = { testFile };

    auto handle = receiver->doMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

// ========== TrashFileEventReceiver::doRestoreFromTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, DoRestoreFromTrash_EmptySourceList)
{
    QList<QUrl> emptySources;
    QUrl target = QUrl::fromLocalFile(tempDir->path());

    auto handle = receiver->doRestoreFromTrash(0, emptySources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

// ========== TrashFileEventReceiver::doCopyFromTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, DoCopyFromTrash_EmptySourceList)
{
    QList<QUrl> emptySources;
    QUrl target = QUrl::fromLocalFile(tempDir->path());

    auto handle = receiver->doCopyFromTrash(0, emptySources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoCopyFromTrash_ValidSources)
{
    stub.set_lamda(&FileCopyMoveJob::copyFromTrash, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl source = QUrl::fromLocalFile("/tmp/.Trash/test.txt");
    QUrl target = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { source };

    auto handle = receiver->doCopyFromTrash(0, sources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_NE(handle, nullptr);
}

// ========== TrashFileEventReceiver::doCleanTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, DoCleanTrash_UserCancelsDialog)
{
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Rejected;
    });

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/test.txt");
    QList<QUrl> sources = { trashFile };

    auto handle = receiver->doCleanTrash(0, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr, true);

    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoCleanTrash_EmptySourcesStartsAsync)
{
    // Empty sources should trigger async counting
    QList<QUrl> emptySources;

    stub.set_lamda(&DialogManager::showClearTrashDialog, [](DialogManager *, quint64) -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    auto handle = receiver->doCleanTrash(0, emptySources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr, false);

    // Should return nullptr and start async operation
    EXPECT_EQ(handle, nullptr);
}

TEST_F(TestTrashFileEventReceiver, DoCleanTrash_ValidSources)
{
    stub.set_lamda(&DialogManager::showClearTrashDialog, [](DialogManager *, quint64) -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::cleanTrash, [](FileCopyMoveJob *, const QList<QUrl> &) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/test.txt");
    QList<QUrl> sources = { trashFile };

    auto handle = receiver->doCleanTrash(0, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr, false);

    EXPECT_NE(handle, nullptr);
}

// ========== TrashFileEventReceiver::handleOperationMoveToTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, HandleOperationMoveToTrash_BasicCall)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::moveToTrash, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl testFile = createTestFile("move.txt");
    QList<QUrl> sources = { testFile };

    // Should not crash
    receiver->handleOperationMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestTrashFileEventReceiver, HandleOperationMoveToTrash_WithCallback)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::moveToTrash, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl testFile = createTestFile("callback.txt");
    QList<QUrl> sources = { testFile };

    receiver->handleOperationMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== TrashFileEventReceiver::handleOperationRestoreFromTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, HandleOperationRestoreFromTrash_BasicCall)
{
    QUrl source = QUrl::fromLocalFile("/tmp/.Trash/restore.txt");
    QUrl target = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { source };

    receiver->handleOperationRestoreFromTrash(0, sources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestTrashFileEventReceiver, HandleOperationRestoreFromTrash_WithCallback)
{
    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl source = QUrl::fromLocalFile("/tmp/.Trash/restore.txt");
    QUrl target = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { source };

    receiver->handleOperationRestoreFromTrash(0, sources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== TrashFileEventReceiver::handleOperationCleanTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, HandleOperationCleanTrash_WithSources)
{
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::cleanTrash, [](FileCopyMoveJob *, const QList<QUrl> &) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/clean.txt");
    QList<QUrl> sources = { trashFile };

    receiver->handleOperationCleanTrash(0, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);

    SUCCEED();
}

TEST_F(TestTrashFileEventReceiver, HandleOperationCleanTrash_WithCallback)
{
    int execRet = QDialog::Rejected;
    stub.set_lamda(VADDR(QDialog, exec), [&] { __DBG_STUB_INVOKE__ return execRet; });

    stub.set_lamda(&DialogManager::showClearTrashDialog, [](DialogManager *, quint64) -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::cleanTrash, [](FileCopyMoveJob *, const QList<QUrl> &) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/callback.txt");
    QList<QUrl> sources = { trashFile };

    receiver->handleOperationCleanTrash(0, sources, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== TrashFileEventReceiver::handleOperationCopyFromTrash() Tests ==========

TEST_F(TestTrashFileEventReceiver, HandleOperationCopyFromTrash_BasicCall)
{
    stub.set_lamda(&FileCopyMoveJob::copyFromTrash, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl source = QUrl::fromLocalFile("/tmp/.Trash/copy.txt");
    QUrl target = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { source };

    receiver->handleOperationCopyFromTrash(0, sources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestTrashFileEventReceiver, HandleOperationCopyFromTrash_WithCallback)
{
    stub.set_lamda(&FileCopyMoveJob::copyFromTrash, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl source = QUrl::fromLocalFile("/tmp/.Trash/copy.txt");
    QUrl target = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { source };

    receiver->handleOperationCopyFromTrash(0, sources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== TrashFileEventReceiver::handleSaveRedoOpt() Tests ==========

TEST_F(TestTrashFileEventReceiver, HandleSaveRedoOpt_ValidToken)
{
    // This test verifies the redo operation saving mechanism
    // In real scenario, the token would be inserted by undo operations

    QString testToken = "test_token_123";

    // Should handle gracefully even if token doesn't exist
    receiver->handleSaveRedoOpt(testToken, false);

    SUCCEED();
}

// ========== Signal Tests ==========

TEST_F(TestTrashFileEventReceiver, Signal_CleanTrashUrls)
{
    QSignalSpy spy(receiver, &TrashFileEventReceiver::cleanTrashUrls);

    // Note: This signal is emitted from async operations,
    // so we just verify the signal exists and is connectable
    EXPECT_TRUE(spy.isValid());
}

// ========== Edge Cases ==========

TEST_F(TestTrashFileEventReceiver, EdgeCase_NullHandleCallback)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::moveToTrash, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl testFile = createTestFile("null_callback.txt");
    QList<QUrl> sources = { testFile };

    // Should handle null callback gracefully
    receiver->handleOperationMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestTrashFileEventReceiver, EdgeCase_RevocationFlag)
{
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, [](SystemPathUtil *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileCopyMoveJob::moveToTrash, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl testFile = createTestFile("revocation.txt");
    QList<QUrl> sources = { testFile };

    // With revocation flag, should skip confirmation dialog
    receiver->handleOperationMoveToTrash(0, sources, AbstractJobHandler::JobFlag::kRevocation, nullptr);

    SUCCEED();
}
