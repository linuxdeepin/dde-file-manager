#include <gtest/gtest.h>
#include <QDateTime>

#define private public
#define protected public

#include "dfilecopymovejob.h"
#include "private/dfilecopymovejob_p.h"
#include "testhelper.h"
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
        std::cout << "start DFileCopyMoveJobTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyMoveJobTest" << std::endl;
    }
};

TEST_F(DFileCopyMoveJobTest,can_job_running) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Pictures/Wallpapers");
    target.setPath("~/test_copy_all");

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    QProcess::execute("mkdir " + target.toLocalFile());
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    urlsour.setPath("/etc/apt");
    job->setFileHints(DFileCopyMoveJob::FollowSymlink);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::Attributes);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::ResizeDestinationFile);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::DontFormatFileName);
    job->setFileHints( job->fileHints() | DFileCopyMoveJob::DontSortInode);
    job->start(DUrlList() << urlsour, target);
    job->togglePause();
    job->togglePause();
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    job->togglePause();
}

TEST_F(DFileCopyMoveJobTest,can_job_running_cut) {
    job->setMode(DFileCopyMoveJob::CutMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("/etc/apt");
    target.setPath("~/test_copy_all/etc");
    QProcess::execute("mkdir " + target.toLocalFile());
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);

    EXPECT_EQ(target.toString(),job->targetUrl().toString());
    EXPECT_EQ(-1,job->totalDataSize());
    EXPECT_EQ(0,job->totalFilesCount());

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
    urlsour.setPath("~/test_copy_all/etc");
    target.setPath("~/test_copy_all/etc1");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);


    while(!job->isFinished()) {
        QThread::msleep(100);
    }

    target.setPath("~/test_copy_all");
    QProcess::execute("rm -rf " + target.toLocalFile());
}

TEST_F(DFileCopyMoveJobTest,can_job_running_remove) {
    job->setMode(DFileCopyMoveJob::CutMode);
    job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    EXPECT_EQ(DFileCopyMoveJob::ForceDeleteFile,job->fileHints());
    urlsour.setPath("~/test_copy_all");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_error) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Wallpapers");
    target.setPath("/usr/bin");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {
        QThread::msleep(100);
    }
}

TEST_F(DFileCopyMoveJobTest,get_errorToString) {
    DFileCopyMoveJobPrivate * jobd = reinterpret_cast<DFileCopyMoveJobPrivate *>(qGetPtrHelper(job->d_ptr.data()));
    ASSERT_TRUE(jobd);
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::PermissionError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::SpecialFileError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::FileExistsError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::DirectoryExistsError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::OpenError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::ReadError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::WriteError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::MkdirError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::RemoveError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::RenameError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::FileSizeTooBigError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::NotEnoughSpaceError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::IntegrityCheckingError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::TargetReadOnlyError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::TargetIsSelfError).isEmpty());
    EXPECT_FALSE(jobd->errorToString(DFileCopyMoveJob::UnknowError).isEmpty());
}
