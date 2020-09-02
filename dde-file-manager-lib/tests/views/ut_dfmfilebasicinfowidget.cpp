#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include <views/dfmfilebasicinfowidget.h>
#include <views/dfmfilebasicinfowidget.cpp>
#include <QStandardPaths>

using namespace testing;
DFM_BEGIN_NAMESPACE
namespace  {
    class DFMFileBasicInfoWidgetTest : public Test
    {
    public:
        virtual void SetUp() override{
            m_biw = new DFMFileBasicInfoWidget(nullptr);
        }

        virtual void TearDown() override{
            delete  m_biw;
        }
    public:
        DFMFileBasicInfoWidget *m_biw;
    };

}


TEST_F(DFMFileBasicInfoWidgetTest, setUrl){

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    EXPECT_TRUE(m_biw->d_func()->m_url.isEmpty());
    m_biw->setUrl(desktopUrl);
    auto expectValue = m_biw->d_func()->m_url == desktopUrl;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, showFileName){
    auto expectValue = m_biw->showFileName() == m_biw->d_func()->m_showFileName;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowFileName){
    m_biw->setShowFileName(false);
    auto expectValue = false == m_biw->d_func()->m_showFileName;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, showMediaInfo){
    auto expectValue = m_biw->showMediaInfo() == m_biw->d_func()->m_showMediaInfo;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowMediaInfo){
    m_biw->setShowMediaInfo(false);
    auto expectValue = false == m_biw->d_func()->m_showMediaInfo;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, showSummary){
    auto expectValue = m_biw->showSummary() == m_biw->d_func()->m_showSummaryOnly;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, setShowSummary){
    m_biw->setShowSummary(false);
    auto expectValue = false == m_biw->d_func()->m_showSummaryOnly;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMFileBasicInfoWidgetTest, updateSizeText){
//    auto expectValue = m_biw->showFileName() == m_biw->d_func()->m_showFileName;
//    EXPECT_TRUE(expectValue);
}

DFM_END_NAMESPACE
