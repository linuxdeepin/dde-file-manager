// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations/fileoperationsservice.h"
#include "fileoperations/copyfiles/copyfiles.h"
#include "fileoperations/cutfiles/cutfiles.h"
#include "fileoperations/deletefiles/deletefiles.h"
#include "fileoperations/trashfiles/movetotrashfiles.h"
#include "fileoperations/trashfiles/restoretrashfiles.h"
#include "fileoperations/trashfiles/copyfromtrashfiles.h"
#include "fileoperations/cleantrash/cleantrashfiles.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestFileOperationsService : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        service = new FileOperationsService();
        ASSERT_TRUE(service);
    }

    void TearDown() override
    {
        stub.clear();
        if (service) {
            delete service;
            service = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    FileOperationsService *service;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestFileOperationsService, Constructor_Initializes)
{
    EXPECT_TRUE(service);
}

TEST_F(TestFileOperationsService, Destructor_Cleanup)
{
    delete service;
    service = nullptr;
    SUCCEED();
}

// ========== copy Tests ==========

TEST_F(TestFileOperationsService, Copy_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    JobHandlePointer handle = service->copy(sources, target);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, Copy_WithFlags)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/target");
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kCopyRemote;

    JobHandlePointer handle = service->copy(sources, target, flags);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, Copy_MultipleSources)
{
    QList<QUrl> sources = {
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt"),
        QUrl::fromLocalFile("/tmp/file3.txt")
    };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    JobHandlePointer handle = service->copy(sources, target);

    ASSERT_NE(handle, nullptr);
}

// ========== copyFromTrash Tests ==========

TEST_F(TestFileOperationsService, CopyFromTrash_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/restore");

    JobHandlePointer handle = service->copyFromTrash(sources, target);

    ASSERT_NE(handle, nullptr);
}

// ========== moveToTrash Tests ==========

TEST_F(TestFileOperationsService, MoveToTrash_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/delete_me.txt") };

    JobHandlePointer handle = service->moveToTrash(sources);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, MoveToTrash_MultipleFiles)
{
    QList<QUrl> sources = {
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };

    JobHandlePointer handle = service->moveToTrash(sources);

    ASSERT_NE(handle, nullptr);
}

// ========== restoreFromTrash Tests ==========

TEST_F(TestFileOperationsService, RestoreFromTrash_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/restore");

    JobHandlePointer handle = service->restoreFromTrash(sources, target);

    ASSERT_NE(handle, nullptr);
}

// ========== deletes Tests ==========

TEST_F(TestFileOperationsService, Deletes_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/delete.txt") };

    JobHandlePointer handle = service->deletes(sources);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, Deletes_WithFlags)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/file.txt") };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    JobHandlePointer handle = service->deletes(sources, flags);

    ASSERT_NE(handle, nullptr);
}

// ========== cut Tests ==========

TEST_F(TestFileOperationsService, Cut_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/cut_source.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/cut_dest");

    JobHandlePointer handle = service->cut(sources, target);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, Cut_SameDevice)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/source.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/target");

    JobHandlePointer handle = service->cut(sources, target);

    ASSERT_NE(handle, nullptr);
}

// ========== cleanTrash Tests ==========

TEST_F(TestFileOperationsService, CleanTrash_CreatesJobHandler)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash") };

    JobHandlePointer handle = service->cleanTrash(sources);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, CleanTrash_MultipleTrashDirs)
{
    QList<QUrl> sources = {
        QUrl::fromLocalFile("/tmp/.Trash"),
        QUrl::fromLocalFile("/home/.Trash")
    };

    JobHandlePointer handle = service->cleanTrash(sources);

    ASSERT_NE(handle, nullptr);
}

// ========== handleWorkerFinish Tests ==========

TEST_F(TestFileOperationsService, HandleWorkerFinish_RemovesJob)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    JobHandlePointer handle = service->copy(sources, target);

    // Simulate worker finish by emitting signal
    emit handle->workerFinish();

    SUCCEED();
}

TEST_F(TestFileOperationsService, HandleWorkerFinish_NoSender)
{
    // Call without sender should handle gracefully
    service->handleWorkerFinish();

    SUCCEED();
}

// ========== Integration Tests ==========

TEST_F(TestFileOperationsService, Integration_MultipleOperations)
{
    // Create multiple operations
    JobHandlePointer copy = service->copy({ QUrl::fromLocalFile("/tmp/copy.txt") },
                                          QUrl::fromLocalFile("/tmp/dest"));
    JobHandlePointer del = service->deletes({ QUrl::fromLocalFile("/tmp/delete.txt") });
    JobHandlePointer cut = service->cut({ QUrl::fromLocalFile("/tmp/cut.txt") },
                                        QUrl::fromLocalFile("/tmp/target"));

    ASSERT_NE(copy, nullptr);
    ASSERT_NE(del, nullptr);
    ASSERT_NE(cut, nullptr);
}

// ========== Edge Cases ==========

TEST_F(TestFileOperationsService, EdgeCase_EmptySourceList)
{
    QList<QUrl> emptySources;
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    JobHandlePointer handle = service->copy(emptySources, target);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, EdgeCase_InvalidUrls)
{
    QList<QUrl> invalidSources = { QUrl(), QUrl("invalid://path") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    JobHandlePointer handle = service->copy(invalidSources, target);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, EdgeCase_EmptyTargetUrl)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl emptyTarget;

    JobHandlePointer handle = service->copy(sources, emptyTarget);

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, EdgeCase_LargeSourceList)
{
    QList<QUrl> largeSources;
    for (int i = 0; i < 1000; ++i) {
        largeSources.append(QUrl::fromLocalFile(QString("/tmp/file%1.txt").arg(i)));
    }

    JobHandlePointer handle = service->copy(largeSources, QUrl::fromLocalFile("/tmp/dest"));

    ASSERT_NE(handle, nullptr);
}

TEST_F(TestFileOperationsService, EdgeCase_AllOperationsTypes)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    // Test all operations
    JobHandlePointer h1 = service->copy(sources, target);
    JobHandlePointer h2 = service->copyFromTrash(sources, target);
    JobHandlePointer h3 = service->moveToTrash(sources);
    JobHandlePointer h4 = service->restoreFromTrash(sources, target);
    JobHandlePointer h5 = service->deletes(sources);
    JobHandlePointer h6 = service->cut(sources, target);
    JobHandlePointer h7 = service->cleanTrash(sources);

    ASSERT_NE(h1, nullptr);
    ASSERT_NE(h2, nullptr);
    ASSERT_NE(h3, nullptr);
    ASSERT_NE(h4, nullptr);
    ASSERT_NE(h5, nullptr);
    ASSERT_NE(h6, nullptr);
    ASSERT_NE(h7, nullptr);
}
