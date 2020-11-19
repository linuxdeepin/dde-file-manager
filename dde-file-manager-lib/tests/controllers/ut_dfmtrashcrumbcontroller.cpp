#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmtrashcrumbcontroller.h"

#define private public
using DFM_NAMESPACE::DFMTrashCrumbController;



namespace  {
class DFMTrashCrumbControllerTest : public testing::Test
{
public:
    virtual void SetUp() override
    {
        controller = new DFMTrashCrumbController;
    }

    virtual void TearDown() override
    {
        delete controller;
        controller = nullptr;
    }

    DFMTrashCrumbController *controller;
};
}


TEST_F(DFMTrashCrumbControllerTest,tSupportedUrl)
{
    DUrl url;
    url.setScheme(TRASH_SCHEME);
    EXPECT_TRUE(controller->supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller->supportedUrl(url));
}

