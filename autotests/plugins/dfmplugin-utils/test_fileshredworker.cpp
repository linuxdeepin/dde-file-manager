// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

