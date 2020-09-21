#include <gtest/gtest.h>
#include <QDateTime>

#include "dfilecopymovejob.h"
#include "dfmglobal.h"
#include <QThread>
#include <QProcess>

using namespace testing;
DFM_USE_NAMESPACE

class DFileCopyMoveJobTest:public testing::Test{

public:

    QSharedPointer<DFileCopyMoveJob> job;
    virtual void SetUp() override{
        job.reset(new DFileCopyMoveJob());
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};

TEST_F(DFileCopyMoveJobTest,can_job_running) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Pictures/Wallpapers");
    target.setPath("~/");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_norefine) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::NoRefine);

    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}
TEST_F(DFileCopyMoveJobTest,can_job_running_MoreThreadRefine) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::MoreThreadRefine);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test/etc");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_MoreThreadAndMainRefine) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::MoreThreadAndMainRefine);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test/etc1");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    QThread::msleep(100);
    job->togglePause();
    QThread::msleep(100);
    job->togglePause();
    QThread::msleep(100);
    job->stop();
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_MoreThreadAndMainAndReadRefine) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::MoreThreadAndMainAndReadRefine);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test/etc2");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_cut) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test/etc4");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(),job->targetUrl().toString());
    EXPECT_EQ(-1,job->totalDataSize());
    EXPECT_EQ(-1,job->totalFilesCount());

    QThread::msleep(300);
    EXPECT_EQ(true,job->totalDataSize() != -1);
    EXPECT_EQ(true,job->totalFilesCount() != -1);
    EXPECT_EQ(DFileCopyMoveJob::NoError,job->error());
    EXPECT_EQ(true,job->isCanShowProgress());

    job->stop();

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_MoreThreadAndMainAndOpenRefine) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setRefine(DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test/etc5");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(),job->targetUrl().toString());
    EXPECT_EQ(-1,job->totalDataSize());
    EXPECT_EQ(-1,job->totalFilesCount());

    QThread::msleep(300);
    EXPECT_EQ(true,job->totalDataSize() != -1);
    EXPECT_EQ(true,job->totalFilesCount() != -1);
    EXPECT_EQ(DFileCopyMoveJob::NoError,job->error());
    EXPECT_EQ(true,job->isCanShowProgress());

    job->stop();

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_refine) {
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/test/etc2");
    target.setPath("~/test/etc3");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);


    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_more) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/test/etc2");
    target.setPath("~/test/etc3");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(),job->targetUrl().toString());
    EXPECT_EQ(-1,job->totalDataSize());
    EXPECT_EQ(-1,job->totalFilesCount());

    QThread::msleep(300);
    EXPECT_EQ(true,job->totalDataSize() != -1);
    EXPECT_EQ(true,job->totalFilesCount() != -1);
    EXPECT_EQ(DFileCopyMoveJob::NoError,job->error());
    EXPECT_EQ(false,job->isCanShowProgress());

    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_remove) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    EXPECT_EQ(DFileCopyMoveJob::ForceDeleteFile,job->fileHints());
    urlsour.setPath("~/Wallpapers");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_remove_all) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    DUrl urlsour;
    urlsour.setScheme(FILE_SCHEME);
    urlsour.setPath("~/test");

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}
