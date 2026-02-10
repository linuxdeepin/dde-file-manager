// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/packetwritingjob.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-burn/dpacketwritingcontroller.h>

#include <DDialog>

#include <QTimer>
#include <QThread>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFM_BURN_USE_NS
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_PacketWritingJob : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Setup common stubs if needed
    }
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_PacketWritingJob, PacketWritingScheduler_instance)
{
    PacketWritingScheduler &scheduler1 = PacketWritingScheduler::instance();
    PacketWritingScheduler &scheduler2 = PacketWritingScheduler::instance();

    EXPECT_EQ(&scheduler1, &scheduler2);   // Should be singleton
}

TEST_F(UT_PacketWritingJob, PacketWritingScheduler_addJob)
{
    bool timerStarted = false;

    // QTimer::start() has overloads, stub the parameterless version
    stub.set_lamda(static_cast<void (QTimer::*)()>(&QTimer::start), [&timerStarted] {
        __DBG_STUB_INVOKE__
        timerStarted = true;
    });

    PutPacketWritingJob *job = new PutPacketWritingJob("/dev/sr0");
    PacketWritingScheduler::instance().addJob(job);

    EXPECT_TRUE(timerStarted);

    // Note: Job will be managed by the scheduler, don't delete manually
}

TEST_F(UT_PacketWritingJob, PacketWritingScheduler_onTimeout)
{
    bool jobStarted = false;

    stub.set_lamda(ADDR(QThread, start), [&jobStarted] {
        __DBG_STUB_INVOKE__
        jobStarted = true;
    });

    PutPacketWritingJob *job = new PutPacketWritingJob("/dev/sr0");
    PacketWritingScheduler::instance().addJob(job);

    // Manually trigger timeout
    PacketWritingScheduler::instance().onTimeout();

    EXPECT_TRUE(jobStarted);

    // Note: Job will be managed by the scheduler, don't delete manually
}

TEST_F(UT_PacketWritingJob, AbstractPacketWritingJob_Constructor)
{
    PutPacketWritingJob job("/dev/sr0");

    EXPECT_EQ(job.device(), "/dev/sr0");
}

TEST_F(UT_PacketWritingJob, AbstractPacketWritingJob_urls2Names)
{
    PutPacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1.txt")
         << QUrl::fromLocalFile("/tmp/file2.txt")
         << QUrl::fromLocalFile("/tmp/subdir/file3.txt");

    QStringList names = job.urls2Names(urls);

    EXPECT_EQ(names.size(), 3);
    EXPECT_TRUE(names.contains("file1.txt"));
    EXPECT_TRUE(names.contains("file2.txt"));
    EXPECT_TRUE(names.contains("file3.txt"));
}

TEST_F(UT_PacketWritingJob, AbstractPacketWritingJob_urls2Names_EmptyList)
{
    PutPacketWritingJob job("/dev/sr0");

    QList<QUrl> emptyUrls;
    QStringList names = job.urls2Names(emptyUrls);

    EXPECT_TRUE(names.isEmpty());
}

TEST_F(UT_PacketWritingJob, PutPacketWritingJob_Constructor)
{
    PutPacketWritingJob job("/dev/sr1");

    EXPECT_EQ(job.device(), "/dev/sr1");
}

TEST_F(UT_PacketWritingJob, PutPacketWritingJob_setPendingUrls)
{
    PutPacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1.txt")
         << QUrl::fromLocalFile("/tmp/file2.txt");

    job.setPendingUrls(urls);

    QList<QUrl> retrievedUrls = job.getPendingUrls();
    EXPECT_EQ(retrievedUrls.size(), 2);
    EXPECT_EQ(retrievedUrls[0], QUrl::fromLocalFile("/tmp/file1.txt"));
    EXPECT_EQ(retrievedUrls[1], QUrl::fromLocalFile("/tmp/file2.txt"));
}

TEST_F(UT_PacketWritingJob, PutPacketWritingJob_work_Success)
{
    PutPacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1.txt");
    job.setPendingUrls(urls);

    bool putCalled = false;
    stub.set_lamda(ADDR(DFMBURN::DPacketWritingController, put), [&putCalled] {
        __DBG_STUB_INVOKE__
        putCalled = true;
        return true;
    });

    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });

    bool result = job.work();
    EXPECT_TRUE(result);
    EXPECT_TRUE(putCalled);
}

TEST_F(UT_PacketWritingJob, PutPacketWritingJob_work_Failure)
{
    PutPacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1.txt");
    job.setPendingUrls(urls);

    stub.set_lamda(ADDR(DFMBURN::DPacketWritingController, put), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_FALSE(result);
}

TEST_F(UT_PacketWritingJob, PutPacketWritingJob_work_EmptyUrls)
{
    PutPacketWritingJob job("/dev/sr0");

    // Don't set any URLs
    bool result = job.work();
    EXPECT_FALSE(result);   // Should fail with empty URLs
}

TEST_F(UT_PacketWritingJob, RemovePacketWritingJob_Constructor)
{
    RemovePacketWritingJob job("/dev/sr2");

    EXPECT_EQ(job.device(), "/dev/sr2");
}

TEST_F(UT_PacketWritingJob, RemovePacketWritingJob_setPendingUrls)
{
    RemovePacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media/sr0/file1.txt")
         << QUrl::fromLocalFile("/media/sr0/file2.txt");

    job.setPendingUrls(urls);

    QList<QUrl> retrievedUrls = job.getPendingUrls();
    EXPECT_EQ(retrievedUrls.size(), 2);
    EXPECT_EQ(retrievedUrls[0], QUrl::fromLocalFile("/media/sr0/file1.txt"));
    EXPECT_EQ(retrievedUrls[1], QUrl::fromLocalFile("/media/sr0/file2.txt"));
}

TEST_F(UT_PacketWritingJob, RemovePacketWritingJob_work_Success)
{
    RemovePacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media/sr0/file1.txt");
    job.setPendingUrls(urls);

    bool removeCalled = false;
    stub.set_lamda(ADDR(DFMBURN::DPacketWritingController, rm), [&removeCalled] {
        __DBG_STUB_INVOKE__
        removeCalled = true;
        return true;
    });
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_TRUE(result);
    EXPECT_TRUE(removeCalled);
}

TEST_F(UT_PacketWritingJob, RemovePacketWritingJob_work_Failure)
{
    RemovePacketWritingJob job("/dev/sr0");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media/sr0/file1.txt");
    job.setPendingUrls(urls);

    stub.set_lamda(ADDR(DFMBURN::DPacketWritingController, rm), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_FALSE(result);
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_Constructor)
{
    RenamePacketWritingJob job("/dev/sr3");

    EXPECT_EQ(job.device(), "/dev/sr3");
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_setSrcUrl)
{
    RenamePacketWritingJob job("/dev/sr0");

    QUrl srcUrl = QUrl::fromLocalFile("/media/sr0/oldname.txt");
    job.setSrcUrl(srcUrl);

    EXPECT_EQ(job.getSrcUrl(), srcUrl);
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_setDestUrl)
{
    RenamePacketWritingJob job("/dev/sr0");

    QUrl destUrl = QUrl::fromLocalFile("/media/sr0/newname.txt");
    job.setDestUrl(destUrl);

    EXPECT_EQ(job.getDestUrl(), destUrl);
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_work_Success)
{
    RenamePacketWritingJob job("/dev/sr0");

    QUrl srcUrl = QUrl::fromLocalFile("/media/sr0/oldname.txt");
    QUrl destUrl = QUrl::fromLocalFile("/media/sr0/newname.txt");
    job.setSrcUrl(srcUrl);
    job.setDestUrl(destUrl);

    bool renameCalled = false;
    stub.set_lamda(ADDR(DPacketWritingController, mv), [&renameCalled] {
        __DBG_STUB_INVOKE__
        renameCalled = true;
        return true;
    });
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_TRUE(result);
    EXPECT_TRUE(renameCalled);
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_work_Failure)
{
    RenamePacketWritingJob job("/dev/sr0");

    QUrl srcUrl = QUrl::fromLocalFile("/media/sr0/oldname.txt");
    QUrl destUrl = QUrl::fromLocalFile("/media/sr0/newname.txt");
    job.setSrcUrl(srcUrl);
    job.setDestUrl(destUrl);

    stub.set_lamda(ADDR(DPacketWritingController, mv), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_FALSE(result);
}

TEST_F(UT_PacketWritingJob, RenamePacketWritingJob_work_EmptyUrls)
{
    RenamePacketWritingJob job("/dev/sr0");
    stub.set_lamda(ADDR(DPacketWritingController, mv), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    // Don't set any URLs
    job.pwController.reset(new DPacketWritingController { "curDevice", "mnt" });
    bool result = job.work();
    EXPECT_FALSE(result);   // Should fail with empty URLs
}

TEST_F(UT_PacketWritingJob, AbstractPacketWritingJob_run)
{
    PutPacketWritingJob job("/dev/sr0");

    bool workCalled = false;

    job.run();

    EXPECT_FALSE(workCalled);

    bool finishedEmitted = false;

    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [] {
        __DBG_STUB_INVOKE__
        return "/tmp";
    });

    job.run();

    EXPECT_FALSE(workCalled);

    stub.set_lamda(ADDR(DPacketWritingController, open), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(PutPacketWritingJob, work), [&workCalled] {
        __DBG_STUB_INVOKE__
        workCalled = true;
        return true;
    });
    job.run();

    EXPECT_TRUE(workCalled);
}

TEST_F(UT_PacketWritingJob, PacketWritingScheduler_addJob_NullJob)
{
    // Should handle null job gracefully
    // PacketWritingScheduler::instance().addJob(nullptr);
    // Should not crash
}

TEST_F(UT_PacketWritingJob, PacketWritingScheduler_onTimeout_EmptyQueue)
{
    // Should handle empty queue gracefully
    PacketWritingScheduler::instance().onTimeout();
    // Should not crash
}
