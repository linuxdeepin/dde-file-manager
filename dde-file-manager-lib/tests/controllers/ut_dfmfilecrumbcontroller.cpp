#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#define private public
#include <QDesktopServices>
#include "controllers/dfmfilecrumbcontroller.h"
DFM_USE_NAMESPACE



namespace  {

class TestDFMFileCrumbController : public testing::Test
{
public:
    DFMFileCrumbController *dfmFileCrubCtl;
    virtual void SetUp() override
    {
        dfmFileCrubCtl = new DFMFileCrumbController();
    }

    virtual void TearDown() override
    {
        delete dfmFileCrubCtl;
        dfmFileCrubCtl = nullptr;
    }
};

}

TEST_F(TestDFMFileCrumbController, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_TRUE(dfmFileCrubCtl->supportedUrl(url));
}

TEST_F(TestDFMFileCrumbController, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl->seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}

TEST_F(TestDFMFileCrumbController, tst_seprateUrl2)
{
    DUrl url("file:///tmp");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl->seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
TEST_F(TestDFMFileCrumbController, tst_seprateUrl3)
{
    DUrl url("file:///");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl->seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
TEST_F(TestDFMFileCrumbController, tst_seprateUrl4)
{
    QString filename;
    DUrl url("file:///media");
    QList<CrumbData> crumbDataList = dfmFileCrubCtl->seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}

TEST_F(TestDFMFileCrumbController, tst_getDisplayName)
{
    QProcess::execute("toutch /tmp/1234.txt");
    QString filename("/tmp/1234.txt");
    QString getFilename = dfmFileCrubCtl->getDisplayName(filename);
    QProcess::execute("rm /tmp/1234.txt -f");
}
