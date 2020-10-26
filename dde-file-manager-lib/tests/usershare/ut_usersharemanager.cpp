#include <gtest/gtest.h>
#include <QDateTime>

#include "usershare/usersharemanager.h"
#include <QProcess>
#include <QDebug>

using namespace testing;
class UserShareManagerTest:public testing::Test{

public:
    UserShareManager sharemanager;
    virtual void SetUp() override{
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};
