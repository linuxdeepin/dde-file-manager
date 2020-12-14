#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmtrashcrumbcontroller.h"

using DFM_NAMESPACE::DFMTrashCrumbController;

class TestDFMTrashCrumbController : public testing::Test
{
public:
    DFMTrashCrumbController controller;

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {

    }
};

TEST_F(TestDFMTrashCrumbController,tSupportedUrl)
{
    DUrl url;
    url.setScheme(TRASH_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

