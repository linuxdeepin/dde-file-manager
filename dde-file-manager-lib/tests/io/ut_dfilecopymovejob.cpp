#include <gtest/gtest.h>
#include <QDateTime>

#include "dfilecopymovejob.h"

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

}
