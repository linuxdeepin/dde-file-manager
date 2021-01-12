#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/dfmbookmarkcrumbcontroller.h"

DFM_USE_NAMESPACE



namespace  {

class TestDfmbookmarkcrumbcontroller : public testing::Test
{
public:
    DFMBookmarkCrumbController *dfmBookmarkCrubCtl;
    virtual void SetUp() override
    {
        dfmBookmarkCrubCtl = new DFMBookmarkCrumbController();
    }

    virtual void TearDown() override
    {
        delete dfmBookmarkCrubCtl;
        dfmBookmarkCrubCtl = nullptr;
    }
};

}

TEST_F(TestDfmbookmarkcrumbcontroller, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_FALSE(dfmBookmarkCrubCtl->supportedUrl(url));
}

TEST_F(TestDfmbookmarkcrumbcontroller, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmBookmarkCrubCtl->seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
