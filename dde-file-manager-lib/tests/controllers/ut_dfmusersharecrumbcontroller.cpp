#include <gtest/gtest.h>
#include <QDateTime>
#include <dfmevent.h>

#include "controllers/dfmusersharecrumbcontroller.h"

using namespace testing;
DFM_USE_NAMESPACE

class DFMUserShareCrumbControllerTest:public testing::Test{

public:
    DFMUserShareCrumbController controller;
    virtual void SetUp() override{
        std::cout << "start DFMUserShareCrumbControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFMUserShareCrumbControllerTest" << std::endl;
    }
};


TEST_F(DFMUserShareCrumbControllerTest,can_supportedUrl){
    DUrl url;
    url.setScheme(USERSHARE_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

TEST_F(DFMUserShareCrumbControllerTest,can_seprateUrl){
    DUrl url;
    url.setScheme(USERSHARE_SCHEME);
    EXPECT_TRUE(controller.seprateUrl(url).isEmpty());
}
