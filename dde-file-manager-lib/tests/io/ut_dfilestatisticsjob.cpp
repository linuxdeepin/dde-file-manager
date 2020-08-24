#include <gtest/gtest.h>
#include <QDateTime>

#include "dfilestatisticsjob.h"


using namespace testing;
DFM_USE_NAMESPACE

class DFileStatisticsJobTest:public testing::Test{

public:

    DFileStatisticsJob *job = nullptr;
    DUrl url;
    virtual void SetUp() override{
        job = new DFileStatisticsJob();
        url.setScheme(FILE_SCHEME);
        url.setPath("/");
        std::cout << "start DFileStatisticsJobTest" << std::endl;
    }

    virtual void TearDown() override{
        if (job) {
            delete job;
            job = nullptr;
        }
        std::cout << "end DFileStatisticsJobTest" << std::endl;
    }
};

TEST_F(DFileStatisticsJobTest,can_propty) {
    job->setFileHints(DFileStatisticsJob::ExcludeSourceFile |
                      DFileStatisticsJob::SingleDepth);
    EXPECT_EQ(DFileStatisticsJob::StoppedState, job->state());
    EXPECT_EQ(true, job->fileHints().testFlag(DFileStatisticsJob::ExcludeSourceFile));
    EXPECT_TRUE(job->totalSize() == 0);
    EXPECT_TRUE(job->totalProgressSize() == 0);
    EXPECT_TRUE(job->filesCount() == 0);
    EXPECT_TRUE(job->directorysCount() == 0);
    EXPECT_TRUE(job->directorysCount(false) == 0);
}

TEST_F(DFileStatisticsJobTest,can_running) {

    job->stop();
    job->togglePause();
    url.setPath("/etc/sane.d");
    job->start(DUrlList() << url);
    while (job->filesCount() < 10) {

    }
    job->togglePause();
    int i = 0;
    while(i > 10000) {
        job->state();
        i++;
    }
    job->togglePause();
    while (!job->isFinished()) {

    }
    EXPECT_TRUE(job->totalSize() != 0);
    EXPECT_TRUE(job->totalProgressSize() != 0);
    EXPECT_TRUE(job->filesCount() != 0);
    EXPECT_TRUE(job->directorysCount() != 0);
    EXPECT_TRUE(job->directorysCount(false) != 0);

    job->togglePause();
    job->stop();

}

TEST_F(DFileStatisticsJobTest,can_running_single) {
    job->setFileHints(DFileStatisticsJob::ExcludeSourceFile |
                      DFileStatisticsJob::SingleDepth);
    DUrl tmp;
    tmp.setScheme(FILE_SCHEME);
    tmp.setPath("~/Pictures/");
    url.setPath("/proc/");
    job->start(DUrlList() << url << tmp);

    while (!job->isFinished()) {

    }
    job->stop();
}

TEST_F(DFileStatisticsJobTest,can_running_ExcludeSourceFile) {
    job->setFileHints(DFileStatisticsJob::ExcludeSourceFile | DFileStatisticsJob::FollowSymlink);
    DUrl tmp;
    tmp.setScheme(FILE_SCHEME);
    tmp.setPath("~/Pictures/Colorful-Abstraction01.jpg");
    url.setPath("~/Pictures/");
    DUrl tmp1;
    tmp1.setScheme(FILE_SCHEME);
    tmp1.setPath("/usr/lib32/libc.so.6");

    job->start(DUrlList() << url << tmp << tmp1);
    while (!job->isFinished()) {

    }
    job->stop();
}
