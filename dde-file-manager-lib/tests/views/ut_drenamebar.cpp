#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


#define private public

#include <views/drenamebar.h>
#include <views/drenamebar.cpp>
using namespace testing;
namespace  {
    class RecordRenameBarStateTest : public testing::Test
    {
    public:
        RecordRenameBarStateTest():Test(){}
        virtual void SetUp() override{
        }
        virtual void TearDown()override{
        }
    public:
        RecordRenameBarState rrbs;
    };
}


TEST_F(RecordRenameBarStateTest, setPatternFirstItemContent){
    QPair<QString, QString> testData("aa", "bb");
    rrbs.setPatternFirstItemContent(testData);
    auto expectValue = rrbs.m_patternFirst == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, setPatternSecondItemContent){
    QPair<QString, DFileService::AddTextFlags> testData("aa", DFileService::AddTextFlags::After);
    rrbs.setPatternSecondItemContent(testData);
    auto expectValue = rrbs.m_patternSecond == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, setPatternThirdItemContent){
    QPair<QString, QString> testData("aa", "bb");
    rrbs.setPatternThirdItemContent(testData);
    auto expectValue = rrbs.m_patternThird == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, setbuttonStateInThreePattern){
    std::array<bool, 3> testData{true, true, true};
    rrbs.setbuttonStateInThreePattern(testData);
    auto expectValue = rrbs.m_buttonStateInThreePattern == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, setCurrentPattern){
    std::size_t testData = 10;
    rrbs.setCurrentPattern(testData);
    auto expectValue = rrbs.m_currentPattern == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, setSelectedUrls){
    QList<DUrl> testData{DUrl("aa"), DUrl("bb")};
    rrbs.setSelectedUrls(testData);
    auto expectValue = rrbs.m_selectedUrls == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getPatternFirstItemContent){
    auto expectValue = rrbs.m_patternFirst == rrbs.getPatternFirstItemContent();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getPatternSecondItemContent){
    auto expectValue = rrbs.m_patternSecond == rrbs.getPatternSecondItemContent();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getPatternThirdItemContent){
    auto expectValue = rrbs.m_patternThird == rrbs.getPatternThirdItemContent();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getbuttonStateInThreePattern){
    auto expectValue = rrbs.m_buttonStateInThreePattern == rrbs.getbuttonStateInThreePattern();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getCurrentPattern){
    auto expectValue = rrbs.m_currentPattern == rrbs.getCurrentPattern();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getSelectedUrl){
    auto expectValue = rrbs.m_selectedUrls == rrbs.getSelectedUrl();
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, getVisibleValue){
    auto expectValue = rrbs.m_visible == rrbs.getVisibleValue();
    EXPECT_TRUE(expectValue);
}

TEST(DRenameBarTest, storeUrlList){
    DRenameBar drb;
    QList<DUrl> testData{DUrl("aa"), DUrl("bb")};
    drb.storeUrlList(testData);
    auto expectValue = drb.d_func()->m_urlList == testData;
    EXPECT_TRUE(expectValue);
}

TEST(DRenameBarTest, resetRenameBar){
    DRenameBar drb;
    drb.resetRenameBar();
    EXPECT_TRUE(0 == drb.d_func()->m_currentPattern);
}

TEST(DRenameBarTest, getCurrentState){
    DRenameBar drb;
    auto temp = drb.getCurrentState();
    auto expectValue = temp->getCurrentPattern() == drb.d_func()->m_currentPattern;
    EXPECT_TRUE(expectValue);
}

TEST(DRenameBarTest, loadState){
    std::unique_ptr<RecordRenameBarState>  renameBarState{ nullptr };
    DRenameBar drb;
    drb.loadState(renameBarState);

    auto expectValue = 0 == drb.d_func()->m_currentPattern;
    EXPECT_TRUE(expectValue);
}

TEST(DRenameBarTest, setVisible){
    DRenameBar drb;
    drb.setVisible(true);
    EXPECT_TRUE(drb.isVisible());
}

