#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/dfmavfscrumbcontroller.h"

DFM_USE_NAMESPACE



namespace  {

class TestDfmavfscrumbcontroller : public testing::Test
{
public:
    DFMAvfsCrumbController dfmAvsCtl;
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestDfmavfscrumbcontroller, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_FALSE(dfmAvsCtl.supportedUrl(url));
}
