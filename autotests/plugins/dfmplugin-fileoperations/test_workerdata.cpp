// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QFileDevice>

#include "stubext.h"

#include "fileoperations/fileoperationutils/workerdata.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestWorkerData : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Register file info factories
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ========== WorkerData Constructor Tests ==========

TEST_F(TestWorkerData, Constructor_InitializesDefaultValues)
{
    WorkerData data;

    EXPECT_EQ(data.dirSize, 0);
    EXPECT_EQ(data.jobFlags, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_FALSE(data.exBlockSyncEveryWrite);
    EXPECT_FALSE(data.isBlockDevice);
    EXPECT_FALSE(data.isSourceFileLocal);
    EXPECT_FALSE(data.isTargetFileLocal);
    EXPECT_EQ(data.currentWriteSize, 0);
    EXPECT_EQ(data.zeroOrlinkOrDirWriteSize, 0);
    EXPECT_EQ(data.blockRenameWriteSize, 0);
    EXPECT_EQ(data.skipWriteSize, 0);
    EXPECT_EQ(data.completeFileCount, 0);
    EXPECT_TRUE(data.singleThread);
}

// ========== WorkerData::BlockFileCopyInfo Tests ==========

TEST_F(TestWorkerData, BlockFileCopyInfo_DefaultConstructor)
{
    WorkerData::BlockFileCopyInfo info;

    EXPECT_TRUE(info.closeflag);
    EXPECT_FALSE(info.isdir);
    EXPECT_EQ(info.frominfo, nullptr);
    EXPECT_EQ(info.toinfo, nullptr);
    EXPECT_EQ(info.buffer, nullptr);
    EXPECT_EQ(info.size, 0);
    EXPECT_EQ(info.currentpos, 0);
    EXPECT_EQ(info.permission, QFileDevice::ReadOwner);
}

TEST_F(TestWorkerData, BlockFileCopyInfo_CopyConstructor)
{
    WorkerData::BlockFileCopyInfo original;
    original.closeflag = false;
    original.isdir = true;
    original.size = 1024;
    original.currentpos = 512;
    original.permission = QFileDevice::WriteOwner;

    WorkerData::BlockFileCopyInfo copy(original);

    EXPECT_EQ(copy.closeflag, false);
    EXPECT_EQ(copy.isdir, true);
    EXPECT_EQ(copy.size, 1024);
    EXPECT_EQ(copy.currentpos, 512);
    EXPECT_EQ(copy.permission, QFileDevice::WriteOwner);
}

TEST_F(TestWorkerData, BlockFileCopyInfo_BufferManagement)
{
    WorkerData::BlockFileCopyInfo *info = new WorkerData::BlockFileCopyInfo();

    // Allocate buffer
    info->buffer = new char[1024];
    info->size = 1024;

    EXPECT_NE(info->buffer, nullptr);

    // Destructor should clean up buffer
    delete info;

    SUCCEED();
}

TEST_F(TestWorkerData, BlockFileCopyInfo_OpenFlagInitialization)
{
    WorkerData::BlockFileCopyInfo info;

    // Should be initialized with O_CREAT | O_WRONLY | O_TRUNC
    int expectedFlags = O_CREAT | O_WRONLY | O_TRUNC;
    EXPECT_EQ(info.openFlag.loadAcquire(), expectedFlags);
}

// ========== WorkerData Atomic Fields Tests ==========

TEST_F(TestWorkerData, AtomicFields_ExBlockSyncEveryWrite)
{
    WorkerData data;

    EXPECT_FALSE(data.exBlockSyncEveryWrite);

    data.exBlockSyncEveryWrite = true;
    EXPECT_TRUE(data.exBlockSyncEveryWrite);

    data.exBlockSyncEveryWrite = false;
    EXPECT_FALSE(data.exBlockSyncEveryWrite);
}

TEST_F(TestWorkerData, AtomicFields_IsBlockDevice)
{
    WorkerData data;

    data.isBlockDevice = true;
    EXPECT_TRUE(data.isBlockDevice);
}

TEST_F(TestWorkerData, AtomicFields_IsSourceFileLocal)
{
    WorkerData data;

    data.isSourceFileLocal = true;
    EXPECT_TRUE(data.isSourceFileLocal);
}

TEST_F(TestWorkerData, AtomicFields_IsTargetFileLocal)
{
    WorkerData data;

    data.isTargetFileLocal = true;
    EXPECT_TRUE(data.isTargetFileLocal);
}

TEST_F(TestWorkerData, AtomicFields_CurrentWriteSize)
{
    WorkerData data;

    data.currentWriteSize = 2048;
    EXPECT_EQ(data.currentWriteSize, 2048);

    data.currentWriteSize += 512;
    EXPECT_EQ(data.currentWriteSize, 2560);
}

TEST_F(TestWorkerData, AtomicFields_ZeroOrlinkOrDirWriteSize)
{
    WorkerData data;

    data.zeroOrlinkOrDirWriteSize = 1024;
    EXPECT_EQ(data.zeroOrlinkOrDirWriteSize.loadAcquire(), 1024);

    data.zeroOrlinkOrDirWriteSize.fetchAndAddOrdered(512);
    EXPECT_EQ(data.zeroOrlinkOrDirWriteSize.loadAcquire(), 1536);
}

TEST_F(TestWorkerData, AtomicFields_BlockRenameWriteSize)
{
    WorkerData data;

    data.blockRenameWriteSize = 4096;
    EXPECT_EQ(data.blockRenameWriteSize.loadAcquire(), 4096);
}

TEST_F(TestWorkerData, AtomicFields_SkipWriteSize)
{
    WorkerData data;

    data.skipWriteSize = 8192;
    EXPECT_EQ(data.skipWriteSize.loadAcquire(), 8192);
}

TEST_F(TestWorkerData, AtomicFields_CompleteFileCount)
{
    WorkerData data;

    EXPECT_EQ(data.completeFileCount.loadAcquire(), 0);

    data.completeFileCount.fetchAndAddOrdered(1);
    EXPECT_EQ(data.completeFileCount.loadAcquire(), 1);

    data.completeFileCount.fetchAndAddOrdered(5);
    EXPECT_EQ(data.completeFileCount.loadAcquire(), 6);
}

TEST_F(TestWorkerData, AtomicFields_SingleThread)
{
    WorkerData data;

    EXPECT_TRUE(data.singleThread);

    data.singleThread = false;
    EXPECT_FALSE(data.singleThread);
}

// ========== WorkerData Maps/Queues Tests ==========

TEST_F(TestWorkerData, ErrorOfAction_EmptyByDefault)
{
    WorkerData data;

    EXPECT_TRUE(data.errorOfAction.isEmpty());
}

TEST_F(TestWorkerData, ErrorOfAction_InsertAndRetrieve)
{
    WorkerData data;

    data.errorOfAction[AbstractJobHandler::JobErrorType::kPermissionError] = AbstractJobHandler::SupportAction::kSkipAction;
    data.errorOfAction[AbstractJobHandler::JobErrorType::kProrogramError] = AbstractJobHandler::SupportAction::kRetryAction;

    EXPECT_EQ(data.errorOfAction.size(), 2);
    EXPECT_EQ(data.errorOfAction[AbstractJobHandler::JobErrorType::kPermissionError], AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_EQ(data.errorOfAction[AbstractJobHandler::JobErrorType::kProrogramError], AbstractJobHandler::SupportAction::kRetryAction);
}

TEST_F(TestWorkerData, EveryFileWriteSize_ThreadSafe)
{
    WorkerData data;

    QUrl file1 = QUrl::fromLocalFile("/tmp/file1.txt");
    QUrl file2 = QUrl::fromLocalFile("/tmp/file2.txt");

    data.everyFileWriteSize.insert(file1, 1024);
    data.everyFileWriteSize.insert(file2, 2048);

    EXPECT_TRUE(data.everyFileWriteSize.contains(file1));
    EXPECT_TRUE(data.everyFileWriteSize.contains(file2));
}

// ========== WorkerData JobFlags Tests ==========

TEST_F(TestWorkerData, JobFlags_DefaultIsNoHint)
{
    WorkerData data;

    EXPECT_EQ(data.jobFlags, AbstractJobHandler::JobFlag::kNoHint);
}

TEST_F(TestWorkerData, JobFlags_CanBeSet)
{
    WorkerData data;

    data.jobFlags = AbstractJobHandler::JobFlag::kCopyFollowSymlink;
    EXPECT_EQ(data.jobFlags, AbstractJobHandler::JobFlag::kCopyFollowSymlink);

    data.jobFlags = AbstractJobHandler::JobFlag::kCopyToSelf;
    EXPECT_EQ(data.jobFlags, AbstractJobHandler::JobFlag::kCopyToSelf);
}

// ========== BlockFileCopyInfo FileInfo Tests ==========

TEST_F(TestWorkerData, BlockFileCopyInfo_FileInfoPointers)
{
    WorkerData::BlockFileCopyInfo info;

    QUrl fromUrl = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl toUrl = QUrl::fromLocalFile("/tmp/dest.txt");

    info.frominfo = InfoFactory::create<FileInfo>(fromUrl);
    info.toinfo = InfoFactory::create<FileInfo>(toUrl);

    // Should be able to hold FileInfo pointers
    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestWorkerData, EdgeCase_MultipleInstances)
{
    WorkerData data1;
    WorkerData data2;

    data1.dirSize = 10;
    data2.dirSize = 20;

    EXPECT_EQ(data1.dirSize, 10);
    EXPECT_EQ(data2.dirSize, 20);

    // Each instance should be independent
    EXPECT_NE(data1.dirSize, data2.dirSize);
}

TEST_F(TestWorkerData, EdgeCase_LargeFileSizes)
{
    WorkerData data;

    qint64 largeSize = 10LL * 1024 * 1024 * 1024;   // 10GB

    data.currentWriteSize = largeSize;
    EXPECT_EQ(data.currentWriteSize, largeSize);

    data.zeroOrlinkOrDirWriteSize = largeSize;
    EXPECT_EQ(data.zeroOrlinkOrDirWriteSize.loadAcquire(), largeSize);
}

TEST_F(TestWorkerData, EdgeCase_MaxCompleteFileCount)
{
    WorkerData data;

    for (int i = 0; i < 10000; ++i) {
        data.completeFileCount.fetchAndAddOrdered(1);
    }

    EXPECT_EQ(data.completeFileCount.loadAcquire(), 10000);
}

TEST_F(TestWorkerData, EdgeCase_BufferLargeAllocation)
{
    WorkerData::BlockFileCopyInfo *info = new WorkerData::BlockFileCopyInfo();

    // Allocate 1MB buffer
    info->buffer = new char[1024 * 1024];
    info->size = 1024 * 1024;

    EXPECT_NE(info->buffer, nullptr);

    delete info;   // Should clean up without crash

    SUCCEED();
}

TEST_F(TestWorkerData, EdgeCase_NullBufferInDestructor)
{
    WorkerData::BlockFileCopyInfo *info = new WorkerData::BlockFileCopyInfo();

    // Buffer is null by default
    EXPECT_EQ(info->buffer, nullptr);

    // Destructor should handle null buffer gracefully
    delete info;

    SUCCEED();
}
