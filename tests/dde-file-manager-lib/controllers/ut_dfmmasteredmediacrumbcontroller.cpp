#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/dfmmasteredmediacrumbcontroller.h"

DFM_USE_NAMESPACE



namespace  {

class TestDFMMasteredMediaCrumbController : public testing::Test
{
public:
    DFMMasteredMediaCrumbController dfmMasteredMediaCrumbCtl;
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestDFMMasteredMediaCrumbController, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_FALSE(dfmMasteredMediaCrumbCtl.supportedUrl(url));
}

TEST_F(TestDFMMasteredMediaCrumbController, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmMasteredMediaCrumbCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}

TEST_F(TestDFMMasteredMediaCrumbController, tst_seprateUrl2)
{
    DUrl url("file:///tmp");
    QList<CrumbData> crumbDataList = dfmMasteredMediaCrumbCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
