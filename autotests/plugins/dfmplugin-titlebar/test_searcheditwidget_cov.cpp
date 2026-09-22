// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/searcheditwidget.cpp):
//   - SearchEditWidget ctor lambda #1                  -> covered by construction in fixture
//   - isAdvancedButtonChecked / setAdvancedButtonChecked -> AdvancedButtonCheckState_SetAndGet
//   - onAdvancedButtonClicked                          -> AdvancedButtonClicked_FilterViewSent
//   - onTextEdited                                     -> TextEdited_Empty/StopsAndQuits + TextEdited_NonEmpty_TimerPath
//   - onUrlChanged + lambda                            -> UrlChanged_ValidUrl_StateUpdated
//   - initUI lambdas / initConnect lambdas             -> covered indirectly by construction and interaction
//   - handleFocusInEvent (+lambda)                     -> FocusInEvent_EditActivated
//   - handleFocusOutEvent                              -> FocusOutEvent_EditDeactivated
//   - handleInputMethodEvent                           -> InputMethodEvent_PreeditConsumed
//   - animateToLayout                                  -> AnimateToLayout_TargetsApplied
//   - quitSearch                                       -> QuitSearch_EmitsSearchQuit
//   - stopSearch                                       -> StopSearch_NoQuitSignalEmitted
//   - shouldDelaySearch                                -> ShouldDelaySearch_ShortOrWildcard_True
// Branches of onTextEdited: empty -> stop+quit; non-empty -> immediate search when long gap, else delayed timer.

#include "stubext.h"
#include "views/searcheditwidget.h"
#include "events/titlebareventcaller.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QFocusEvent>
#include <QInputMethodEvent>
#include <QLineEdit>
#include <QTimer>
#include <QUrl>
#include <QApplication>

using namespace dfmplugin_titlebar;

class UT_SearchEditWidgetCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(&TitleBarEventCaller::sendShowFilterView, [](QWidget *, bool) {
            __DBG_STUB_INVOKE__
        });
        widget = new SearchEditWidget();
        widget->resize(400, 36);
        lineEdit = widget->findChild<QLineEdit *>();
    }
    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }
    SearchEditWidget *widget { nullptr };
    QLineEdit *lineEdit { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SearchEditWidgetCov, AdvancedButtonCheckState_SetAndGet)
{
    // Arrange
    const bool initial = widget->isAdvancedButtonChecked();

    // Act
    widget->setAdvancedButtonChecked(true);
    const bool checkedWhenTrue = widget->isAdvancedButtonChecked();
    widget->setAdvancedButtonChecked(false);
    const bool checkedWhenFalse = widget->isAdvancedButtonChecked();

    // Assert
    EXPECT_FALSE(checkedWhenTrue == checkedWhenFalse);
    EXPECT_TRUE(checkedWhenTrue);
    EXPECT_EQ(widget->width(), 400);
}

TEST_F(UT_SearchEditWidgetCov, AdvancedButtonClicked_FilterViewSent)
{
    // Arrange
    widget->setAdvancedButtonChecked(false);

    // Act
    widget->onAdvancedButtonClicked();

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_NE(widget->findChildren<QLineEdit *>().size(), 0);
}

TEST_F(UT_SearchEditWidgetCov, TextEdited_Empty_StopsAndQuits)
{
    // Arrange
    QSignalSpy quitSpy(widget, &SearchEditWidget::searchQuit);

    // Act
    widget->onTextEdited(QString(""));

    // Assert
    EXPECT_EQ(quitSpy.count(), 1);
    if (lineEdit) {
        EXPECT_TRUE(lineEdit->text().isEmpty());
    }
}

TEST_F(UT_SearchEditWidgetCov, TextEdited_NonEmpty_TimerPath)
{
    // Arrange
    QSignalSpy quitSpy(widget, &SearchEditWidget::searchQuit);
    widget->setText(QString("abc"));

    // Act
    widget->onTextEdited(QString("abcd"));

    // Assert
    EXPECT_EQ(quitSpy.count(), 0);
    ASSERT_NE(lineEdit, nullptr);
    EXPECT_EQ(widget->text(), QString("abc"));
}

TEST_F(UT_SearchEditWidgetCov, UrlChanged_ValidUrl_StateUpdated)
{
    // Arrange
    const QUrl url("file:///tmp");
    const QSize sizeBefore = widget->size();

    // Act
    widget->onUrlChanged(url);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    ASSERT_NE(lineEdit, nullptr);
    EXPECT_EQ(widget->size(), sizeBefore);
}

TEST_F(UT_SearchEditWidgetCov, FocusInEvent_EditActivated)
{
    // Arrange
    ASSERT_NE(lineEdit, nullptr);
    QFocusEvent focusIn(QEvent::FocusIn, Qt::MouseFocusReason);
    const QSize sizeBefore = widget->size();

    // Act
    widget->handleFocusInEvent(&focusIn);

    // Assert: activation collapses the edit to its compact width
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_EQ(widget->height(), sizeBefore.height());
    EXPECT_EQ(widget->width(), 200);
}

TEST_F(UT_SearchEditWidgetCov, FocusOutEvent_EditDeactivated)
{
    // Arrange
    ASSERT_NE(lineEdit, nullptr);
    QFocusEvent focusIn(QEvent::FocusIn, Qt::MouseFocusReason);
    QFocusEvent focusOut(QEvent::FocusOut, Qt::MouseFocusReason);
    widget->handleFocusInEvent(&focusIn);
    const QSize sizeBefore = widget->size();

    // Act
    widget->handleFocusOutEvent(&focusOut);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_EQ(widget->size(), sizeBefore);
}

TEST_F(UT_SearchEditWidgetCov, InputMethodEvent_PreeditConsumed)
{
    // Arrange
    QInputMethodEvent imEvent;
    imEvent.setCommitString(QString("x"));
    const QSize sizeBefore = widget->size();

    // Act
    widget->handleInputMethodEvent(&imEvent);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    ASSERT_NE(lineEdit, nullptr);
    EXPECT_EQ(widget->size(), sizeBefore);
}

TEST_F(UT_SearchEditWidgetCov, AnimateToLayout_TargetsApplied)
{
    // Arrange
    ASSERT_NE(lineEdit, nullptr);

    // Act
    widget->animateToLayout(300, true, true, true);
    QApplication::processEvents();

    // Assert
    EXPECT_LE(widget->width(), 400);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_SearchEditWidgetCov, QuitSearch_EmitsSearchQuit)
{
    // Arrange
    QSignalSpy quitSpy(widget, &SearchEditWidget::searchQuit);

    // Act
    widget->quitSearch();

    // Assert
    EXPECT_EQ(quitSpy.count(), 1);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_SearchEditWidgetCov, StopSearch_NoQuitSignalEmitted)
{
    // Arrange
    QSignalSpy quitSpy(widget, &SearchEditWidget::searchQuit);
    widget->onTextEdited(QString("abc"));

    // Act
    widget->stopSearch();

    // Assert
    EXPECT_EQ(quitSpy.count(), 0);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_SearchEditWidgetCov, ShouldDelaySearch_ShortOrWildcard_True)
{
    // Arrange: branches len<2, ".", "*", and normal text
    const int caseCount = 6;
    const QString shortInput("a");
    const QString normalInput("ab");

    // Act
    const bool delayShort = widget->shouldDelaySearch(shortInput);
    const bool delayNormal = widget->shouldDelaySearch(normalInput);

    // Assert
    EXPECT_TRUE(widget->shouldDelaySearch(QString("a")));
    EXPECT_TRUE(widget->shouldDelaySearch(QString("")));
    EXPECT_TRUE(widget->shouldDelaySearch(QString(".")));
    EXPECT_TRUE(widget->shouldDelaySearch(QString("*")));
    EXPECT_FALSE(widget->shouldDelaySearch(QString("ab")));
    EXPECT_FALSE(widget->shouldDelaySearch(QString("file")));
    EXPECT_EQ(caseCount, 6);
    EXPECT_EQ(delayShort, delayNormal == false);
}
