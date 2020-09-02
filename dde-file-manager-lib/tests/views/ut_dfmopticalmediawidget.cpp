#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include <views/dfmopticalmediawidget.h>
#include <views/dfmopticalmediawidget.cpp>

using namespace testing;

namespace  {
    class DFMOpticalMediaWidgetTest : public Test
    {
    public:
        virtual void SetUp() override {
            m_omw = new DFMOpticalMediaWidget(nullptr);
        }

        virtual void TearDown() override {
            delete m_omw;
        }
    public:
        DFMOpticalMediaWidget *m_omw;
    };
}



TEST_F(DFMOpticalMediaWidgetTest, updateDiscInfo){
    m_omw->updateDiscInfo("Disc_AA");
    auto expectValue = m_omw->d_func()->curdev == "Disc_AA";
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, setDiscMountPoint){
    m_omw->setDiscMountPoint("home/");
    auto expectValue = m_omw->d_func()->strMntPath == "home/";
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, hasFileInDir){
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    auto expectValue = true == m_omw->hasFileInDir(QDir(desktopPath));
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMOpticalMediaWidgetTest, getDiscMountPoint){
    auto expectValue = m_omw->getDiscMountPoint() == m_omw->d_func()->strMntPath;
    EXPECT_TRUE(expectValue);
}



