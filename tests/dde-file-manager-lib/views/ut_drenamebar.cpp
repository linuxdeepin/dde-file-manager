/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "stub.h"

#define private public
#define protected public

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


TEST(RecordRenameBarState, test_move_structure)
{
    RecordRenameBarState older;
    QPair<QString, QString> testData("aa", "bb");
    older.setPatternFirstItemContent(testData);

    RecordRenameBarState newer(std::move(older));
    auto expectValue = newer.m_patternFirst == testData;
    EXPECT_TRUE(expectValue);
}

TEST(RecordRenameBarState, test_operator_equal)
{
    RecordRenameBarState older;
    QPair<QString, QString> testData("aa", "bb");
    older.setPatternFirstItemContent(testData);

    RecordRenameBarState newer;
    newer = older;
    auto expectValue = newer.m_patternFirst == testData;
    EXPECT_TRUE(expectValue);
}

TEST(RecordRenameBarState, test_operator_move)
{
    RecordRenameBarState older;
    QPair<QString, QString> testData("aa", "bb");
    older.setPatternFirstItemContent(testData);

    RecordRenameBarState newer =std::move(older);
    auto expectValue = newer.m_patternFirst == testData;
    EXPECT_TRUE(expectValue);
}

TEST_F(RecordRenameBarStateTest, test_clear){
    DUrlList tp;
    tp.append(DUrl("/home/"));
    rrbs.setSelectedUrls(tp);
    rrbs.clear();
    auto expectValue = 0 == rrbs.m_selectedUrls.size();
    EXPECT_TRUE(expectValue);
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

TEST(DRenameBarPrivate, set_rename_btn_status)
{
    DRenameBar drb;

    QPushButton *button{ std::get<1>(drb.d_func()->m_buttonsArea) };
    button->setEnabled(false);
    drb.d_func()->setRenameBtnStatus(true);
    EXPECT_TRUE(button->isEnabled());
}

TEST(DRenameBarPrivate, filtering_text)
{
    DRenameBar drb;

    auto tempEmpty = drb.d_func()->filteringText("");
    EXPECT_TRUE(tempEmpty.isEmpty());

    auto temp = drb.d_func()->filteringText("\\\\:test\\\\");
    EXPECT_TRUE("test" == temp);
}

TEST(DRenameBarPrivate, update_line_edit_text)
{
    DRenameBar drb;

    QLineEdit *lineEdit{ std::get<1>(drb.d_func()->m_replaceOperatorItems) };
    drb.d_func()->updateLineEditText(lineEdit);
    EXPECT_TRUE(lineEdit->text().isEmpty());

    auto temp = drb.d_func()->filteringText("\\\\:test\\\\");
    lineEdit->setText(temp);
    drb.d_func()->updateLineEditText(lineEdit);
    EXPECT_TRUE("test" == temp);
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
    std::unique_ptr<RecordRenameBarState>  renameBarState2 {drb.getCurrentState()};
    drb.loadState(renameBarState2);
    auto expectValue2 = 0 == drb.d_func()->m_urlList.size();
    EXPECT_TRUE(expectValue2);
}

TEST(DRenameBarTest, setVisible){
    DRenameBar drb;
    drb.setVisible(true);
    EXPECT_TRUE(drb.isVisible());
}

static bool inThere = false;
TEST(DRenameBarTest, key_press_event){
    DRenameBar drb;
    QKeyEvent keA(QEvent::MouseButtonPress, Qt::Key_A, Qt::NoModifier);
    QKeyEvent keReturn(QEvent::MouseButtonPress, Qt::Key_Return, Qt::NoModifier);
    QKeyEvent keEnter(QEvent::MouseButtonPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keEscape(QEvent::MouseButtonPress, Qt::Key_Escape, Qt::NoModifier);

    Stub stub;
    auto ut_stubClickCancelButton = (void(*)())([]{
        inThere = true;
    });
    auto ut_stubFunClickRenameButton = (void(*)())([]{
        inThere = true;
    });
    stub.set(ADDR(DRenameBar, clickRenameButton), ut_stubFunClickRenameButton);
    stub.set(ADDR(DRenameBar, clickCancelButton), ut_stubClickCancelButton);
    drb.keyPressEvent(&keA);
    EXPECT_FALSE(inThere);
    inThere = false;
    drb.keyPressEvent(&keEnter);
    EXPECT_TRUE(inThere);

    inThere = false;
    drb.keyPressEvent(&keReturn);
    EXPECT_TRUE(inThere);

    inThere = false;
    drb.keyPressEvent(&keEscape);
    EXPECT_TRUE(inThere);
}


TEST(DRenameBarTest, on_custom_operator_file_name_changed) {
    DRenameBar drb;
    QLineEdit *lineEditForFileName{ std::get<1>(drb.d_func()->m_customOPeratorItems) };
    lineEditForFileName->setText("");
    drb.onCustomOperatorFileNameChanged();
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[2]);

    QLineEdit *lineEditForSNNumber{ std::get<3>(drb.d_func()->m_customOPeratorItems) };
    lineEditForFileName->setText("test");
    lineEditForSNNumber->setText("test");
    drb.onCustomOperatorFileNameChanged();
    EXPECT_TRUE(drb.d_func()->m_renameButtonStates[2]);

    lineEditForSNNumber->setText("");
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[2]);
}

TEST(DRenameBarTest, on_custom_operator_sNNumber_changed) {
    DRenameBar drb;
    QLineEdit *lineEditForSNNumber{ std::get<3>(drb.d_func()->m_customOPeratorItems) };
    lineEditForSNNumber->setText("");
    drb.onCustomOperatorSNNumberChanged();
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[2]);

    QLineEdit *lineEditForFileName{ std::get<3>(drb.d_func()->m_customOPeratorItems) };
    lineEditForFileName->setText("test");
    lineEditForSNNumber->setText("test");
    drb.onCustomOperatorSNNumberChanged();
    EXPECT_TRUE(drb.d_func()->m_renameButtonStates[2]);

    lineEditForFileName->setText("");
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[2]);
}

TEST(DRenameBarTest, on_replace_operator_file_name_changed) {
    DRenameBar drb;
    drb.onReplaceOperatorFileNameChanged("");
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[0]);

    drb.onReplaceOperatorFileNameChanged("test");
    EXPECT_TRUE(drb.d_func()->m_renameButtonStates[0]);
}

TEST(DRenameBarTest, on_add_operator_added_content_changed) {
    DRenameBar drb;
    drb.onAddOperatorAddedContentChanged("");
    EXPECT_FALSE(drb.d_func()->m_renameButtonStates[1]);

    drb.onAddOperatorAddedContentChanged("test");
    EXPECT_TRUE(drb.d_func()->m_renameButtonStates[1]);
}

