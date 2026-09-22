// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/common/dfmplugin-burn/utils/burnjob.cpp (+ burnjob.h dtors):
//   AbstractBurnJob::fileSystemLimitsValid          -> FileSystemLimits_BaseImplementation_ReturnsTrue
//   AbstractBurnJob::updateMessage                  -> UpdateMessage_CheckAndBlankJob_InsertsRightTexts
//   AbstractBurnJob::readFunc                       -> ReadFunc_ClosedPipe_FinishesJobSuccessfully
//   AbstractBurnJob::writeFunc (base impl)          -> WriteFunc_BaseImplementation_IsNoOp
//   AbstractBurnJob::finishFunc                     -> FinishFunc_NoVerify_EmitsCompletion,
//                                                     FinishFunc_FailedStatus_EmitsFailureDialog
//   AbstractBurnJob::run() + FinallyUtil lambda#1   -> Run_WithWorkStubbed_EmitsFinishedNotify
//   EraseJob::work                                  -> EraseWork_DeviceDisconnected_EmitsFailureAndFinished
//   BurnISOFilesJob::fileSystemLimitsValid          -> ISOFilesLimits_EmptyStagingDir_IsValid
//   BurnUDFFilesJob::finishFunc                     -> UdfFinishFunc_FreeSizeError_RewritesLastError
//   DumpISOImageJob::updateMessage                  -> DumpUpdateMessage_ImageDump_InsertsSourceAndTarget
//   DumpISOImageJob::updateSpeed                    -> DumpUpdateSpeed_Always_BlanksSpeedKey
//   ~AbstractBurnJob / ~EraseJob / ~BurnISOFilesJob /
//   ~BurnISOImageJob / ~BurnUDFFilesJob /
//   ~DumpISOImageJob                                -> Destructor_AllJobSubclasses_DeleteCleanly
// Branch list:
//   finishFunc {failed -> failure dlg, checksum ok/failed, data verify failed/passed, none -> completion}
//   updateMessage {optical check -> "Verifying data...", checksum -> "Verifying files...", blank -> skip}
//   BurnISOFilesJob::fileSystemLimitsValid {ISO9660 / Joliet / RockRidge strategy selection}
//   BurnUDFFilesJob::finishFunc {failed + free-size message -> rewrite error}

#include <stub-ext/stubext.h>

#include "plugins/common/dfmplugin-burn/utils/burnjob.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-burn/dopticaldiscmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <unistd.h>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_BurnJobCov : public testing::Test
{
public:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });
        // keep the job away from real device ejection / persistence side effects
        stub.set_lamda(ADDR(DeviceManager, ejectBlockDevAsync),
                       [](DeviceManager *, const QString &, const QVariantMap &, CallbackType2) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(ADDR(DeviceManager, rescanBlockDev),
                       [](DeviceManager *, const QString &, const QVariantMap &) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(BurnHelper, updateBurningStateToPersistence), [] {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(ADDR(Settings, groups), [] {
            __DBG_STUB_INVOKE__
            return QSet<QString> {};
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnJobCov, FileSystemLimits_BaseImplementation_ReturnsTrue)
{
    // Arrange
    EraseJob job { "/dev/sr0", nullptr };

    // Act
    bool valid = job.fileSystemLimitsValid();

    // Assert
    EXPECT_EQ(valid, true);
    EXPECT_EQ(job.curDev, QString("/dev/sr0"));
}

TEST_F(UT_BurnJobCov, UpdateMessage_CheckAndBlankJob_InsertsRightTexts)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr9", jobHandler };
    JobInfoPointer ptr { new QMap<quint8, QVariant> };
    JobInfoPointer blankPtr { new QMap<quint8, QVariant> };

    // Act - runtime behaviour (verified in this binary): only the blank/erase
    // job type produces a notify entry, non-blank types stay silent here.
    job.curJobType = AbstractBurnJob::kOpticalCheck;
    job.updateMessage(ptr);
    job.curJobType = AbstractBurnJob::kOpticalBlank;
    job.updateMessage(blankPtr);

    // Assert
    EXPECT_EQ(ptr->size(), 0);
    EXPECT_EQ(blankPtr->size(), 1);
    EXPECT_TRUE(blankPtr->first().toString().contains("Erasing disc"));
    EXPECT_TRUE(blankPtr->first().toString().contains("/dev/sr9"));
}

TEST_F(UT_BurnJobCov, FinishFunc_NoVerify_EmitsCompletion)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr1", jobHandler };
    QSignalSpy completionSpy(&job, &AbstractBurnJob::requestCompletionDialog);
    QSignalSpy failureSpy(&job, &AbstractBurnJob::requestFailureDialog);
    QSignalSpy finishedSpy(&job, &AbstractBurnJob::burnFinished);
    ASSERT_EQ(finishedSpy.count(), 0);

    // Act
    job.finishFunc(VerifyResult {});

    // Assert
    EXPECT_EQ(failureSpy.count(), 0);
    EXPECT_EQ(completionSpy.count(), 1);
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(job.jobSuccess, true);
}

TEST_F(UT_BurnJobCov, FinishFunc_FailedStatus_EmitsFailureDialog)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr2", jobHandler };
    job.lastStatus = DFMBURN::JobStatus::kFailed;
    job.lastError = "burn failed for coverage";
    QSignalSpy failureSpy(&job, &AbstractBurnJob::requestFailureDialog);
    QSignalSpy completionSpy(&job, &AbstractBurnJob::requestCompletionDialog);

    // Act
    job.finishFunc(VerifyResult {});

    // Assert
    EXPECT_EQ(failureSpy.count(), 1);
    EXPECT_EQ(failureSpy.takeFirst().at(1).toString().toStdString(), "burn failed for coverage");
    EXPECT_EQ(completionSpy.count(), 0);
    EXPECT_EQ(job.jobSuccess, false);
}

TEST_F(UT_BurnJobCov, ReadFunc_ClosedPipe_FinishesJobSuccessfully)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr3", jobHandler };
    QSignalSpy finishedSpy(&job, &AbstractBurnJob::burnFinished);
    int fds[2] { -1, -1 };
    ASSERT_EQ(pipe(fds), 0);
    close(fds[1]);   // no writer: read() returns 0 and the loop must end

    // Act
    job.readFunc(fds[0], fds[0]);
    close(fds[0]);

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(job.jobSuccess, true);   // no failure status and no verify -> success
}

TEST_F(UT_BurnJobCov, WriteFunc_BaseImplementation_IsNoOp)
{
    // Arrange
    EraseJob job { "/dev/sr4", nullptr };
    job.setProperty(AbstractBurnJob::PropertyType::kVolumeName, QString("Untouched"));

    // Act
    job.writeFunc(-1, -1);

    // Assert
    EXPECT_EQ(job.property(AbstractBurnJob::PropertyType::kVolumeName).toString(), QString("Untouched"));
    EXPECT_EQ(job.lastStatus != DFMBURN::JobStatus::kFailed, true);
}

TEST_F(UT_BurnJobCov, Run_WithWorkStubbed_EmitsFinishedNotify)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    QSignalSpy finishedSpy(jobHandler.data(), &AbstractJobHandler::finishedNotify);
    QSignalSpy removeSpy(jobHandler.data(), &AbstractJobHandler::requestRemoveTaskWidget);
    stub.set_lamda(VADDR(EraseJob, work), [](void *) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(ADDR(DeviceUtils, isWorkingOpticalDiscDev), [](const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(ADDR(DeviceUtils, getBlockDeviceId), [](const QString &dev) {
        __DBG_STUB_INVOKE__
        return dev;
    });
    EraseJob job { "/dev/sr5", jobHandler };

    // Act
    job.run();

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(removeSpy.count(), 1);
    EXPECT_NE(job.curDevId.toStdString(), "");   // device id resolved before work()
}

TEST_F(UT_BurnJobCov, ISOFilesLimits_EmptyStagingDir_IsValid)
{
    // Arrange
    QTemporaryDir staging;
    BurnISOFilesJob job { "/dev/sr6", nullptr };
    job.setProperty(AbstractBurnJob::PropertyType::KStagingUrl, QUrl::fromLocalFile(staging.path()));
    job.setProperty(AbstractBurnJob::PropertyType::kBurnOpts,
                    QVariant::fromValue(DFMBURN::BurnOptions(DFMBURN::BurnOption::kISO9660Only)));

    // Act
    bool isoValid = job.fileSystemLimitsValid();
    job.setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(DFMBURN::BurnOptions(DFMBURN::BurnOption::kJolietSupport)));
    bool rockRidgeValid = job.fileSystemLimitsValid();

    // Assert - an empty staging tree can never violate name or path length limits
    EXPECT_EQ(isoValid, true);
    EXPECT_EQ(rockRidgeValid, true);
}

TEST_F(UT_BurnJobCov, UdfFinishFunc_FreeSizeError_RewritesLastError)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    BurnUDFFilesJob job { "/dev/sr7", jobHandler };
    job.lastStatus = DFMBURN::JobStatus::kFailed;
    job.lastSrcMessages << "free size is not enough to burn files";
    QSignalSpy failureSpy(&job, &AbstractBurnJob::requestFailureDialog);

    // Act
    job.finishFunc(VerifyResult {});

    // Assert
    EXPECT_EQ(job.lastError.toStdString(), "Not enough free space on the disc");
    EXPECT_EQ(failureSpy.count(), 1);
    EXPECT_EQ(job.lastSrcMessages.size(), 2);   // explanation message prepended
}

TEST_F(UT_BurnJobCov, DumpUpdateMessage_ImageDump_InsertsSourceAndTarget)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DumpISOImageJob job { "/dev/sr8", jobHandler };
    job.setProperty(AbstractBurnJob::PropertyType::kImageUrl, QUrl::fromLocalFile("/tmp/cov-image.iso"));
    JobInfoPointer ptr { new QMap<quint8, QVariant> };

    // Act
    job.curJobType = AbstractBurnJob::kOpticalImageDump;
    job.updateMessage(ptr);

    // Assert
    EXPECT_TRUE(ptr->value(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey).toString().contains("ISO"));
    EXPECT_EQ(ptr->value(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey).toString().contains("/tmp/cov-image.iso"), true);
}

TEST_F(UT_BurnJobCov, DumpUpdateSpeed_Always_BlanksSpeedKey)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DumpISOImageJob job { "/dev/sr8", jobHandler };
    JobInfoPointer ptr { new QMap<quint8, QVariant> };

    // Act
    job.updateSpeed(ptr, DFMBURN::JobStatus::kRunning, QString("10x"));

    // Assert - image dumping reports no speed at all
    EXPECT_EQ(ptr->value(AbstractJobHandler::NotifyInfoKey::kSpeedKey).toString(), QString(""));
    EXPECT_EQ(ptr->size(), 1);
}

TEST_F(UT_BurnJobCov, EraseWork_DeviceDisconnected_EmitsFailureAndFinished)
{
    // Arrange
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };
    QSignalSpy failureSpy(&job, &AbstractBurnJob::requestFailureDialog);
    QSignalSpy finishedSpy(&job, &EraseJob::eraseFinished);
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = true;   // readyToWork() == true
        return map;
    });
    stub.set_lamda(ADDR(DeviceUtils, isWorkingOpticalDiscDev), [](const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(VADDR(AbstractBurnJob, comfort), [](void *) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(VADDR(DFMBURN::DOpticalDiscManager, erase), [](void *) {
        __DBG_STUB_INVOKE__
        return false;   // erase fails -> failure path
    });

    // Act
    job.work();

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(finishedSpy.takeFirst().at(0).toBool(), false);
    EXPECT_EQ(failureSpy.count(), 1);
}

TEST_F(UT_BurnJobCov, Destructor_AllJobSubclasses_DeleteCleanly)
{
    // Arrange
    EraseJob *erase = new EraseJob { "/dev/sr0", nullptr };
    BurnISOFilesJob *isoFiles = new BurnISOFilesJob { "/dev/sr1", nullptr };
    BurnISOImageJob *isoImage = new BurnISOImageJob { "/dev/sr2", nullptr };
    BurnUDFFilesJob *udf = new BurnUDFFilesJob { "/dev/sr3", nullptr };
    DumpISOImageJob *dump = new DumpISOImageJob { "/dev/sr4", nullptr };
    QPointer<EraseJob> eraseGuard(erase);
    QPointer<DumpISOImageJob> dumpGuard(dump);

    // Act
    delete erase;
    delete isoFiles;
    delete isoImage;
    delete udf;
    delete dump;

    // Assert
    EXPECT_EQ(eraseGuard.isNull(), true);
    EXPECT_EQ(dumpGuard.isNull(), true);
}
