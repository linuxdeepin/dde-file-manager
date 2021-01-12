#include <gtest/gtest.h>
#include <QTimer>
#include <QElapsedTimer>

#define protected public
#define private public

#include "controllers/jobcontroller.h"
#include "controllers/filecontroller.h"
#include "dfmevent.h"

using namespace testing;

using namespace testing;

class JobControllerTest:public testing::Test{

public:
    QSharedPointer<JobController> jobcontroller;
    QSharedPointer<FileController> filecontroller;
    virtual void SetUp() override{
        init("~/Desktop",true);
        std::cout << "start JobControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end JobControllerTest" << std::endl;
    }

    void init(const char *path,bool bresetfilecontroller = false) {
        if (bresetfilecontroller) {
            filecontroller.reset(new FileController());
        }
        DUrl url;
        url.fromLocalFile(path);
        bool isgvfs = false;
        QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
        auto director = filecontroller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(),
                                                                                                  QDir::AllEntries, flags, false, isgvfs));
        jobcontroller.reset(new JobController(url,director));
    }
};

TEST_F(JobControllerTest,start_JobController){
    DUrl url;
    url.fromLocalFile("~/Videos");
    jobcontroller->timer = new QElapsedTimer();
    jobcontroller.reset(new JobController(url,QStringList(),QDir::AllEntries));
    EXPECT_EQ(2000,jobcontroller->timeCeiling());
    jobcontroller->setTimeCeiling(10);
    EXPECT_EQ(10,jobcontroller->timeCeiling());
    jobcontroller->setCountCeiling(100);
    EXPECT_EQ(100,jobcontroller->countCeiling());
    jobcontroller->stopAndDeleteLater();
}

TEST_F(JobControllerTest,start_start){
    jobcontroller->setState(JobController::Started);
    jobcontroller->start();
    jobcontroller->setState(JobController::Paused);
    jobcontroller->start();
    jobcontroller->setState(JobController::Stoped);
    jobcontroller->pause();
    jobcontroller->setState(JobController::Started);
    jobcontroller->pause();
    jobcontroller->setState(JobController::Stoped);
    jobcontroller->start();
    while (jobcontroller->isRunning()) {
        jobcontroller->setTimeCeiling(10);
        jobcontroller->setCountCeiling(10);
        jobcontroller->stopAndDeleteLater();
        QThread::msleep(100);
    }
}

TEST_F(JobControllerTest,start_run){
    DUrl url;
    url.fromLocalFile("~/Pictures/Wallpapers");
    init("~/Pictures/Wallpapers",true);
    jobcontroller->start();
    while (jobcontroller->isRunning()) {
        jobcontroller->stop();
        QThread::msleep(100);
    }
    jobcontroller->setCountCeiling(5);
    jobcontroller->start();
    while (jobcontroller->isRunning()) {
        QThread::msleep(100);
    }
    jobcontroller->run();

}
