#include <gtest/gtest.h>
#include <QDateTime>

#include "dfilecopyqueue.h"


using namespace testing;

class DFileCopyQueueTest:public testing::Test{

public:

    DFileCopyQueue<int> queue;
    virtual void SetUp() override{
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};


TEST_F(DFileCopyQueueTest,can_dequeue) {
    queue.enqueue(1);
    EXPECT_EQ(false,queue.isEmpty());
    EXPECT_EQ(1,queue.head());
    EXPECT_EQ(1,queue.dequeue());
    EXPECT_EQ(true,queue.isEmpty());
    queue.clear();
}
