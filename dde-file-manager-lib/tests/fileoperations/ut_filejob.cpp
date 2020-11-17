#include <gtest/gtest.h>
#include <QDateTime>
#include <QSharedPointer>

#include "fileoperations/filejob.h"


using namespace testing;


class FileJobTest:public testing::Test{

public:

    QSharedPointer<FileJob> job;
    virtual void SetUp() override{
        job.reset(new FileJob(FileJob::Trash));
        std::cout << "start FileJobTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end FileJobTest" << std::endl;
    }
};
