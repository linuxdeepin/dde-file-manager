#include <gtest/gtest.h>
#include <interfaces/dfileservices.h>

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
        if (service) {
            service->deleteLater();
        }
        std::cout << "end DFileSeviceTest" << std::endl;
    }
};
