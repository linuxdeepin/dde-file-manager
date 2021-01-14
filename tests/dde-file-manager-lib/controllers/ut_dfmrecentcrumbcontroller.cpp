#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmrecentcrumbcontroller.h"

using DFM_NAMESPACE::DFMRecentCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class DFMRecentCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            controller = new DFMRecentCrumbController;
        }

        virtual void TearDown() override
        {
            delete controller;
            controller = nullptr;
        }

        DFMRecentCrumbController *controller;
    };
}

TEST_F(DFMRecentCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(RECENT_SCHEME);
    EXPECT_TRUE(controller->supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller->supportedUrl(url));
}

TEST_F(DFMRecentCrumbControllerTest, seprateUrl){
     DUrl url;
     url.setScheme(RECENT_SCHEME);
     QList<CrumbData> list = controller->seprateUrl(url);
     EXPECT_TRUE(!list.empty());
}
