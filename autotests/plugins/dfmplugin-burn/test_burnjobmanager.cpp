// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/common/dfmplugin-burn/utils/burnjobmanager.h"
#include "plugins/common/dfmplugin-burn/utils/burnjob.h"
#include "plugins/common/dfmplugin-burn/utils/auditlogjob.h"
#include "plugins/common/dfmplugin-burn/utils/packetwritingjob.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-io/dfileinfo.h>

#include "stubext.h"

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_BurnJobManager : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnJobManager, startEraseDisc)
{
    bool triggerAddTask { false };
    stub.set_lamda(ADDR(DialogManager, addTask), [&triggerAddTask](DialogManager *, const JobHandlePointer task) {
        __DBG_STUB_INVOKE__
        triggerAddTask = true;
        EXPECT_TRUE(task);
    });

    bool triggerInit { false };
    stub.set_lamda(ADDR(BurnJobManager, initBurnJobConnect),
                   [&triggerInit](BurnJobManager *, AbstractBurnJob *job) {
                       __DBG_STUB_INVOKE__
                       triggerInit = true;
                       EXPECT_TRUE(job);
                       EXPECT_TRUE(qobject_cast<EraseJob *>(job));
                       EXPECT_EQ(job->curDev, "/test/dev/sr0");
                   });

    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart]() {
        __DBG_STUB_INVOKE__
        triggerStart = true;
    });
    BurnJobManager::instance()->startEraseDisc("/test/dev/sr0");

    EXPECT_TRUE(triggerAddTask);
    EXPECT_TRUE(triggerInit);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startBurnISOFiles)
{
    bool triggerAddTask { false };
    stub.set_lamda(ADDR(DialogManager, addTask), [&triggerAddTask](DialogManager *, const JobHandlePointer task) {
        __DBG_STUB_INVOKE__
        triggerAddTask = true;
        EXPECT_TRUE(task);
    });

    bool triggerInit { false };
    stub.set_lamda(ADDR(BurnJobManager, initBurnJobConnect),
                   [&triggerInit](BurnJobManager *, AbstractBurnJob *job) {
                       __DBG_STUB_INVOKE__
                       triggerInit = true;
                       EXPECT_TRUE(job);
                       EXPECT_TRUE(qobject_cast<BurnISOFilesJob *>(job));
                       EXPECT_EQ(job->curDev, "/test/dev/sr0");
                   });

    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart](QThread *obj, QThread::Priority) {
        __DBG_STUB_INVOKE__
        triggerStart = true;
        AbstractBurnJob *job { qobject_cast<AbstractBurnJob *>(obj) };
        EXPECT_TRUE(job);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::KStagingUrl).toUrl(),
                  QUrl::fromLocalFile("/tmp"));
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kSpeeds).toInt(),
                  1);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kVolumeName).toString(),
                  "test");
    });

    BurnJobManager::Config conf { "test", 1, {} };
    BurnJobManager::instance()->startBurnISOFiles("/test/dev/sr0",
                                                  QUrl::fromLocalFile("/tmp"), conf);

    EXPECT_TRUE(triggerAddTask);
    EXPECT_TRUE(triggerInit);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startBurnISOImage)
{
    bool triggerAddTask { false };
    stub.set_lamda(ADDR(DialogManager, addTask), [&triggerAddTask](DialogManager *, const JobHandlePointer task) {
        __DBG_STUB_INVOKE__
        triggerAddTask = true;
        EXPECT_TRUE(task);
    });

    bool triggerInit { false };
    stub.set_lamda(ADDR(BurnJobManager, initBurnJobConnect),
                   [&triggerInit](BurnJobManager *, AbstractBurnJob *job) {
                       __DBG_STUB_INVOKE__
                       triggerInit = true;
                       EXPECT_TRUE(job);
                       EXPECT_TRUE(qobject_cast<BurnISOImageJob *>(job));
                       EXPECT_EQ(job->curDev, "/test/dev/sr0");
                   });

    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart](QThread *obj, QThread::Priority) {
        __DBG_STUB_INVOKE__
        triggerStart = true;
        AbstractBurnJob *job { qobject_cast<AbstractBurnJob *>(obj) };
        EXPECT_TRUE(job);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kImageUrl).toUrl(),
                  QUrl::fromLocalFile("/tmp/test.iso"));
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kSpeeds).toInt(),
                  1);
    });

    BurnJobManager::Config conf { {}, 1, {} };
    BurnJobManager::instance()->startBurnISOImage("/test/dev/sr0",
                                                  QUrl::fromLocalFile("/tmp/test.iso"),
                                                  conf);
    EXPECT_TRUE(triggerAddTask);
    EXPECT_TRUE(triggerInit);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startBurnUDFFiles)
{
    bool triggerAddTask { false };
    stub.set_lamda(ADDR(DialogManager, addTask), [&triggerAddTask](DialogManager *, const JobHandlePointer task) {
        __DBG_STUB_INVOKE__
        triggerAddTask = true;
        EXPECT_TRUE(task);
    });

    bool triggerInit { false };
    stub.set_lamda(ADDR(BurnJobManager, initBurnJobConnect),
                   [&triggerInit](BurnJobManager *, AbstractBurnJob *job) {
                       __DBG_STUB_INVOKE__
                       triggerInit = true;
                       EXPECT_TRUE(job);
                       EXPECT_TRUE(qobject_cast<BurnUDFFilesJob *>(job));
                       EXPECT_EQ(job->curDev, "/test/dev/sr0");
                   });
    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart](QThread *obj, QThread::Priority) {
        __DBG_STUB_INVOKE__
        triggerStart = true;
        AbstractBurnJob *job { qobject_cast<AbstractBurnJob *>(obj) };
        EXPECT_TRUE(job);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::KStagingUrl).toUrl(),
                  QUrl::fromLocalFile("/tmp"));
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kSpeeds).toInt(),
                  1);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kVolumeName).toString(),
                  "test");
    });

    BurnJobManager::Config conf { "test", 1, {} };
    BurnJobManager::instance()->startBurnUDFFiles("/test/dev/sr0",
                                                  QUrl::fromLocalFile("/tmp"), conf);

    EXPECT_TRUE(triggerAddTask);
    EXPECT_TRUE(triggerInit);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startDumpISOImage)
{
    bool triggerAddTask { false };
    stub.set_lamda(ADDR(DialogManager, addTask), [&triggerAddTask](DialogManager *, const JobHandlePointer task) {
        __DBG_STUB_INVOKE__
        triggerAddTask = true;
        EXPECT_TRUE(task);
    });

    bool triggerInit { false };
    stub.set_lamda(ADDR(BurnJobManager, initDumpJobConnect),
                   [&triggerInit](BurnJobManager *, AbstractBurnJob *job) {
                       __DBG_STUB_INVOKE__
                       triggerInit = true;
                       EXPECT_TRUE(job);
                       EXPECT_TRUE(qobject_cast<DumpISOImageJob *>(job));
                       EXPECT_EQ(job->curDev, "/test/dev/sr0");
                   });

    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart](QThread *obj, QThread::Priority) {
        __DBG_STUB_INVOKE__
        triggerStart = true;
        AbstractBurnJob *job { qobject_cast<AbstractBurnJob *>(obj) };
        EXPECT_TRUE(job);
        EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kImageUrl).toUrl(),
                  QUrl::fromLocalFile("/tmp/test.iso"));
    });

    BurnJobManager::instance()->startDumpISOImage("/test/dev/sr0",
                                                  QUrl::fromLocalFile("/tmp/test.iso"));

    EXPECT_TRUE(triggerAddTask);
    EXPECT_TRUE(triggerInit);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startAuditLogForCopyFromDisc)
{
    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart] {
        __DBG_STUB_INVOKE__
        triggerStart = true;
    });

    QList<QUrl> srcs;
    QList<QUrl> dests;
    BurnJobManager::instance()->startAuditLogForCopyFromDisc(srcs, dests);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startAuditLogForBurnFiles)
{
    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart] {
        __DBG_STUB_INVOKE__
        triggerStart = true;
    });

    BurnJobManager::instance()->startAuditLogForBurnFiles({}, {}, {});
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startAuditLogForEraseDisc)
{
    bool triggerStart { false };
    stub.set_lamda(ADDR(QThread, start), [&triggerStart] {
        __DBG_STUB_INVOKE__
        triggerStart = true;
    });

    QVariantMap info;
    info[DeviceProperty::kDrive] = "/dev/sr0";
    info[DeviceProperty::kMedia] = "DVD+RW";
    
    BurnJobManager::instance()->startAuditLogForEraseDisc(info, true);
    EXPECT_TRUE(triggerStart);
}

TEST_F(UT_BurnJobManager, startPutFilesToDisc)
{
    bool triggerAddJob { false };
    stub.set_lamda(ADDR(PacketWritingScheduler, addJob), [&triggerAddJob](PacketWritingScheduler *, AbstractPacketWritingJob *job) {
        __DBG_STUB_INVOKE__
        triggerAddJob = true;
        EXPECT_TRUE(job);
        EXPECT_TRUE(qobject_cast<PutPacketWritingJob *>(job));
        EXPECT_EQ(job->device(), "/dev/sr0");
        
        QList<QUrl> urls = job->property("pendingUrls").value<QList<QUrl>>();
        EXPECT_EQ(urls.size(), 2);
        EXPECT_EQ(urls[0], QUrl::fromLocalFile("/tmp/file1.txt"));
        EXPECT_EQ(urls[1], QUrl::fromLocalFile("/tmp/file2.txt"));
    });

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1.txt") << QUrl::fromLocalFile("/tmp/file2.txt");
    
    BurnJobManager::instance()->startPutFilesToDisc("/dev/sr0", urls);
    EXPECT_TRUE(triggerAddJob);
}

TEST_F(UT_BurnJobManager, startRemoveFilesFromDisc)
{
    bool triggerAddJob { false };
    stub.set_lamda(ADDR(PacketWritingScheduler, addJob), [&triggerAddJob](PacketWritingScheduler *, AbstractPacketWritingJob *job) {
        __DBG_STUB_INVOKE__
        triggerAddJob = true;
        EXPECT_TRUE(job);
        EXPECT_TRUE(qobject_cast<RemovePacketWritingJob *>(job));
        EXPECT_EQ(job->device(), "/dev/sr0");
        
        QList<QUrl> urls = job->property("pendingUrls").value<QList<QUrl>>();
        EXPECT_EQ(urls.size(), 2);
        EXPECT_EQ(urls[0], QUrl::fromLocalFile("/media/sr0/file1.txt"));
        EXPECT_EQ(urls[1], QUrl::fromLocalFile("/media/sr0/file2.txt"));
    });

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media/sr0/file1.txt") << QUrl::fromLocalFile("/media/sr0/file2.txt");
    
    BurnJobManager::instance()->startRemoveFilesFromDisc("/dev/sr0", urls);
    EXPECT_TRUE(triggerAddJob);
}

TEST_F(UT_BurnJobManager, startRenameFileFromDisc)
{
    bool triggerAddJob { false };
    stub.set_lamda(ADDR(PacketWritingScheduler, addJob), [&triggerAddJob](PacketWritingScheduler *, AbstractPacketWritingJob *job) {
        __DBG_STUB_INVOKE__
        triggerAddJob = true;
        EXPECT_TRUE(job);
        EXPECT_TRUE(qobject_cast<RenamePacketWritingJob *>(job));
        EXPECT_EQ(job->device(), "/dev/sr0");
        
        QUrl srcUrl = job->property("srcUrl").toUrl();
        QUrl destUrl = job->property("destUrl").toUrl();
        EXPECT_EQ(srcUrl, QUrl::fromLocalFile("/media/sr0/oldname.txt"));
        EXPECT_EQ(destUrl, QUrl::fromLocalFile("/media/sr0/newname.txt"));
    });

    QUrl srcUrl = QUrl::fromLocalFile("/media/sr0/oldname.txt");
    QUrl destUrl = QUrl::fromLocalFile("/media/sr0/newname.txt");
    
    BurnJobManager::instance()->startRenameFileFromDisc("/dev/sr0", srcUrl, destUrl);
    EXPECT_TRUE(triggerAddJob);
}

TEST_F(UT_BurnJobManager, initBurnJobConnect)
{
    EraseJob job("/dev/sr_test", {});
    BurnJobManager::instance()->initBurnJobConnect(&job);

    bool trigger { false };
    {
        stub.set_lamda(ADDR(BurnJobManager, showOpticalJobCompletionDialog), [&trigger] {
            __DBG_STUB_INVOKE__
            trigger = true;
        });
        emit job.requestCompletionDialog({}, {});
        EXPECT_TRUE(trigger);
    }

    {
        trigger = false;
        stub.set_lamda(ADDR(BurnJobManager, showOpticalJobFailureDialog), [&trigger] {
            __DBG_STUB_INVOKE__
            trigger = true;
        });
        emit job.requestFailureDialog({}, {}, {});
        EXPECT_TRUE(trigger);
    }

    {
        trigger = false;
        stub.set_lamda(ADDR(DialogManager, showErrorDialog), [&trigger] {
            __DBG_STUB_INVOKE__
            trigger = true;
        });
        emit job.requestErrorMessageDialog({}, {});
        EXPECT_TRUE(trigger);
    }
}

TEST_F(UT_BurnJobManager, initDumpJobConnect)
{
    DumpISOImageJob job("/dev/sr_test", {});
    BurnJobManager::instance()->initDumpJobConnect(&job);

    bool trigger { false };
    {
        stub.set_lamda(ADDR(BurnJobManager, showOpticalDumpISOSuccessDialog), [&trigger] {
            __DBG_STUB_INVOKE__
            trigger = true;
        });
        emit job.requestOpticalDumpISOSuccessDialog({});
        EXPECT_TRUE(trigger);
    }

    {
        trigger = false;
        stub.set_lamda(ADDR(BurnJobManager, showOpticalDumpISOFailedDialog), [&trigger] {
            __DBG_STUB_INVOKE__
            trigger = true;
        });
        emit job.requestOpticalDumpISOFailedDialog();
        EXPECT_TRUE(trigger);
    }
}

TEST_F(UT_BurnJobManager, deleteStagingDir_NotDir)
{
    stub.set_lamda(ADDR(DFMIO::DFileInfo, attribute), [] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue((int)DFMIO::DFileInfo::AttributeID::kStandardIsFile);
    });

    QUrl notDirFile { QUrl::fromLocalFile("/test/abc") };
    EXPECT_FALSE(BurnJobManager::instance()->deleteStagingDir(notDirFile));
}

TEST_F(UT_BurnJobManager, deleteStagingDir_NotDisc)
{
    stub.set_lamda(ADDR(DFMIO::DFileInfo, attribute), [] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue((int)DFMIO::DFileInfo::AttributeID::kStandardIsDir);
    });

    QUrl file { QUrl::fromLocalFile("/test/abc") };
    EXPECT_FALSE(BurnJobManager::instance()->deleteStagingDir(file));
}

TEST_F(UT_BurnJobManager, deleteStagingDir_Deleted)
{
    stub.set_lamda(ADDR(DFMIO::DFileInfo, attribute), [] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue((int)DFMIO::DFileInfo::AttributeID::kStandardIsDir);
    });

    stub.set_lamda(ADDR(LocalFileHandler, deleteFileRecursive), [] {
        return false;
    });
    QUrl file { QUrl::fromLocalFile("/test/_dev_sr0") };
    EXPECT_FALSE(BurnJobManager::instance()->deleteStagingDir(file));

    stub.set_lamda(ADDR(LocalFileHandler, deleteFileRecursive), [] {
        return true;
    });
    file = QUrl::fromLocalFile("/test/_dev_sr0");
    EXPECT_TRUE(BurnJobManager::instance()->deleteStagingDir(file));
}

TEST_F(UT_BurnJobManager, showOpticalJobCompletionDialog)
{
    bool trigger { false };
    stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
        __DBG_STUB_INVOKE__
        trigger = true;
        return 0;
    });
    BurnJobManager::instance()->showOpticalJobCompletionDialog({}, {});
    EXPECT_TRUE(trigger);
}

TEST_F(UT_BurnJobManager, showOpticalJobFailureDialog)
{
    bool trigger { false };
    stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
        __DBG_STUB_INVOKE__
        trigger = true;
        return 0;
    });
    BurnJobManager::instance()->showOpticalJobFailureDialog({}, {}, {});
    EXPECT_TRUE(trigger);
}

TEST_F(UT_BurnJobManager, showOpticalDumpISOSuccessDialog)
{
    bool trigger { false };
    stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
        __DBG_STUB_INVOKE__
        trigger = true;
        return 0;
    });
    BurnJobManager::instance()->showOpticalDumpISOSuccessDialog({});
    EXPECT_TRUE(trigger);
}

TEST_F(UT_BurnJobManager, showOpticalDumpISOFailedDialog)
{
    bool trigger { false };
    stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
        __DBG_STUB_INVOKE__
        trigger = true;
        return 0;
    });
    BurnJobManager::instance()->showOpticalDumpISOFailedDialog();
    EXPECT_TRUE(trigger);
}
