// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/fileshredworker.h"

#include <QSignalSpy>
#include <QProcess>
#include <QDir>
#include <QFile>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_FileShredWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        worker = new FileShredWorker();
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

    FileShredWorker *worker { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_FileShredWorker, Constructor_InitializesMembers)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(UT_FileShredWorker, stop_SetsShouldStopFlag)
{
    worker->stop();
}

TEST_F(UT_FileShredWorker, shredFile_EmptyList_EmitsFinished)
{
    QSignalSpy finishedSpy(worker, &FileShredWorker::finished);
    QSignalSpy progressSpy(worker, &FileShredWorker::progressUpdated);

    worker->shredFile(QList<QUrl>());

    EXPECT_GE(progressSpy.count(), 1);
    EXPECT_EQ(finishedSpy.count(), 1);

    if (finishedSpy.count() > 0) {
        QList<QVariant> args = finishedSpy.takeFirst();
        EXPECT_TRUE(args.at(0).toBool());
    }
}

TEST_F(UT_FileShredWorker, shredFile_StopRequested_EmitsCancelled)
{
    worker->stop();

    QSignalSpy finishedSpy(worker, &FileShredWorker::finished);

    worker->shredFile({ QUrl::fromLocalFile("/tmp/test.txt") });

    EXPECT_EQ(finishedSpy.count(), 1);

    if (finishedSpy.count() > 0) {
        QList<QVariant> args = finishedSpy.takeFirst();
        EXPECT_FALSE(args.at(0).toBool());
    }
}

TEST_F(UT_FileShredWorker, shredFile_SymLink_RemovesFile)
{
    bool removeCalled = false;

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::remove),
                   [&removeCalled](const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       removeCalled = true;
                       return true;
                   });

    stub.set_lamda(ADDR(QFileInfo, isSymLink),
                   [](QFileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->shredFile({ QUrl::fromLocalFile("/tmp/symlink") });

    EXPECT_TRUE(removeCalled);
}

TEST_F(UT_FileShredWorker, shredFile_ProcessStartFailed_EmitsFailure)
{
    stub.set_lamda(ADDR(QFileInfo, isSymLink),
                   [](QFileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(ADDR(QFileInfo, isDir),
                   [](QFileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(ADDR(QProcess, waitForStarted),
                   [](QProcess *, int) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QSignalSpy finishedSpy(worker, &FileShredWorker::finished);

    worker->shredFile({ QUrl::fromLocalFile("/tmp/test.txt") });

    EXPECT_EQ(finishedSpy.count(), 1);

    if (finishedSpy.count() > 0) {
        QList<QVariant> args = finishedSpy.takeFirst();
        EXPECT_FALSE(args.at(0).toBool());
    }
}


// ===================== PMS sev-2 regression additions =====================

// PMS:333321 大批量文件粉碎时 shred 命令按 kPerMaxCount=50 分批执行，120 个文件应产生 3 次 QProcess::start
TEST_F(UT_FileShredWorker, BUG333321_ShredFile_ManyFiles_BatchesIntoGroupsOf50)
{
    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // 120 个常规文件路径（不要求真实存在：isSymLink/isDir 为假即按普通文件处理）
    const int kTotal = 120;
    QStringList paths;
    for (int i = 0; i < kTotal; ++i)
        paths << tempDir.filePath(QString("shred-%1.dat").arg(i));

    stub.set_lamda(ADDR(QFileInfo, isSymLink), []() { return false; });
    stub.set_lamda(ADDR(QFileInfo, isDir), []() { return false; });

    int startCalls = 0;
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [&startCalls](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) { ++startCalls; });
    stub.set_lamda(ADDR(QProcess, waitForStarted), []() { return true; });
    stub.set_lamda(static_cast<QProcess::ProcessState (QProcess::*)() const>(&QProcess::state),
                   []() { return QProcess::NotRunning; });
    stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                   []() { return 0; });

    QList<QUrl> urls;
    for (const QString &p : paths)
        urls << QUrl::fromLocalFile(p);

    worker->shredFile(urls);

    // 修复前不分批，start 只被调用 1 次；修复后 ceil(120/50)=3 次
    EXPECT_EQ(startCalls, 3);
}

// PMS:333321 恰好 50 个文件边界情况下只应启动 1 个 shred 进程
TEST_F(UT_FileShredWorker, BUG333321_ShredFile_Exactly50Files_SingleBatch)
{
    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    stub.set_lamda(ADDR(QFileInfo, isSymLink), []() { return false; });
    stub.set_lamda(ADDR(QFileInfo, isDir), []() { return false; });

    int startCalls = 0;
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [&startCalls](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) { ++startCalls; });
    stub.set_lamda(ADDR(QProcess, waitForStarted), []() { return true; });
    stub.set_lamda(static_cast<QProcess::ProcessState (QProcess::*)() const>(&QProcess::state),
                   []() { return QProcess::NotRunning; });
    stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                   []() { return 0; });

    QList<QUrl> urls;
    for (int i = 0; i < 50; ++i)
        urls << QUrl::fromLocalFile(tempDir.filePath(QString("edge-%1.dat").arg(i)));

    worker->shredFile(urls);

    EXPECT_EQ(startCalls, 1);
}
