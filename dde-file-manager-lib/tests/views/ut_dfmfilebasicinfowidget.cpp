#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public
#include <views/dfmfilebasicinfowidget.h>
#include <views/dfmfilebasicinfowidget.cpp>
#include <QStandardPaths>
#include <QEvent>
#include <QFile>

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
            m_biw = nullptr;
        }
    public:
        DFMFileBasicInfoWidget *m_biw;
    };

}


TEST_F(DFMFileBasicInfoWidgetTest, setUrl){

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    EXPECT_TRUE(m_biw->d_func()->m_url.isEmpty());
    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->setUrl(desktopUrl);
    auto expectValue = m_biw->d_func()->m_url == desktopUrl;
    EXPECT_TRUE(expectValue);

    QString imageUrl = QString("file://%1/forUtTest.jpg").arg(desktopPath);
    QFile imageFile(imageUrl);
    if (!imageFile.exists())
        imageFile.open(QFile::WriteOnly);
    imageFile.close();

    m_biw->setShowFileName(true);
    m_biw->setShowMediaInfo(true);
    m_biw->d_func()->m_url = DUrl("");
    m_biw->setUrl(DUrl(imageUrl));

    auto expectValue2 = m_biw->d_func()->m_url == DUrl(imageUrl);
    EXPECT_TRUE(expectValue2);

    if (imageFile.link("linkForUtTest.jpg")) {
        QString imageLinkUrl = QString("file://%1/forUtTest.jpg").arg(desktopPath);
        m_biw->setShowFileName(true);
        m_biw->setShowMediaInfo(true);
        m_biw->d_func()->m_url = DUrl("");
        m_biw->setUrl(DUrl(imageLinkUrl));

        auto expectValueLink = m_biw->d_func()->m_url == DUrl(imageLinkUrl);
        EXPECT_TRUE(expectValueLink);
        QFile linkFile(imageLinkUrl);
        if (linkFile.exists())
            linkFile.remove();
    }

    imageFile.remove();
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

//TEST_F(DFMFileBasicInfoWidgetTest, updateSizeText){
//    auto expectValue = m_biw->showFileName() == m_biw->d_func()->m_showFileName;
//    EXPECT_TRUE(expectValue);
//}

TEST_F(DFMFileBasicInfoWidgetTest, startCalcFolderSize){
    m_biw->d_func()->m_showSummaryOnly = true;
    m_biw->d_func()->m_sizeWorker = nullptr;
    m_biw->d_func()->startCalcFolderSize();
    EXPECT_EQ(nullptr, m_biw->d_func()->m_sizeWorker);

//    m_biw->d_func()->m_url = DUrl("/home");
//    m_biw->d_func()->m_showSummaryOnly = false;
//    m_biw->d_func()->startCalcFolderSize();
//    EXPECT_NE(nullptr, m_biw->d_func()->m_sizeWorker);
}

TEST(SectionValueLabel, test_event){
    QEvent et(QEvent::FontChange);
    SectionValueLabel svlab;
    svlab.setText("aa");
    auto lthaa = svlab.width();
    svlab.setText("aaaa");
    svlab.event(&et);
    auto lthaaaa = svlab.width();
    EXPECT_FALSE(lthaa == lthaaaa);
}

TEST(SectionValueLabel, section_value_label){
    QShowEvent se;
    SectionValueLabel svlab;
    svlab.setText("aa");
    auto lthaa = svlab.width();
    svlab.setText("aaaa");
    svlab.showEvent(&se);
    auto lthaaaa = svlab.width();
    EXPECT_FALSE(lthaa == lthaaaa);
}

TEST(LinkSectionValueLabel, link_target_url){
    LinkSectionValueLabel lsvlab;
    lsvlab.m_linkTargetUrl = DUrl("/home/");
    auto expectValue =DUrl("/home/") == lsvlab.linkTargetUrl();
    EXPECT_TRUE(expectValue);
}

TEST(LinkSectionValueLabel, set_link_target_url){
    LinkSectionValueLabel lsvlab;
    lsvlab.setLinkTargetUrl(DUrl("/home/"));
    auto expectValue =DUrl("/home/") == lsvlab.m_linkTargetUrl;
    EXPECT_TRUE(expectValue);
}



DFM_END_NAMESPACE
