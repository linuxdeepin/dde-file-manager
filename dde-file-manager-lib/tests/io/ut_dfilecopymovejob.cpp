#include <gtest/gtest.h>
#include <QDateTime>

#include "dfilecopymovejob.h"
#include "dfmglobal.h"

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
    target.setPath("~/Pictures/");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, target);
    while(!job->isFinished()) {

    }
}

TEST_F(DFileCopyMoveJobTest,can_job_running_remove) {
    job->setMode(DFileCopyMoveJob::CopyMode);
    DUrl urlsour,target;
    urlsour.setScheme(FILE_SCHEME);
    target.setScheme(FILE_SCHEME);
    urlsour.setPath("~/Pictures/Wallpapers(copy)");
    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }
    job->start(DUrlList() << urlsour, DUrl());
    while(!job->isFinished()) {

    }
}
