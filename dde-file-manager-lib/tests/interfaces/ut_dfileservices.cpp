#include <gtest/gtest.h>
#include <interfaces/dfileservices.h>
#include "controllers/appcontroller.h"
#include "controllers/networkcontroller.h"

using namespace testing;
DFM_USE_NAMESPACE

class DFileSeviceTest:public testing::Test{

public:

    DFileService *service = nullptr;
    virtual void SetUp() override{
        service = DFileService::instance();
        std::cout << "start DFileSeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileSeviceTest" << std::endl;
    }
};

TEST_F(DFileSeviceTest, can_isRegisted){
    AppController::instance()->registerUrlHandle();
    EXPECT_TRUE(service->isRegisted(SMB_SCHEME,"",typeid (NetworkController)));
    EXPECT_TRUE(service->isRegisted(TRASH_SCHEME,""));
    DFileService::printStacktrace();
    DFileService::printStacktrace(3);
}
