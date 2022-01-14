/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <QTimer>
#include <QElapsedTimer>

#define protected public
#define private public

#include "controllers/jobcontroller.h"
#include "controllers/filecontroller.h"
#include "dfmevent.h"
#include "testhelper.h"

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

#ifndef __arm__
TEST_F(JobControllerTest,start_JobController){
    TestHelper::runInLoop([](){});
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
#endif

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
