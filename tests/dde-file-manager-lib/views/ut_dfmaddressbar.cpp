/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include <QtTest>
#include <QAction>
#include <QCompleter>
#include <QStandardPaths>
#include <durl.h>
#include <dspinner.h>
#include "views/dcompleterlistview.h"
#include "interfaces/dfmcrumbbar.h"
#include "interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "controllers/searchhistroymanager.h"
#include "stub.h"
#include "testhelper.h"

#define private public
#define protected public

#include "views/dfmaddressbar.h"
DFM_USE_NAMESPACE

namespace  {
    class DFMAddressBarTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_bar = new DFMAddressBar;
        }

        virtual void TearDown() override
        {
            if (m_bar) {
                delete m_bar;
                m_bar = nullptr;
            }
        }
        DFMAddressBar *m_bar = nullptr;
    };
}

TEST_F(DFMAddressBarTest,get_current_completer)
{
    ASSERT_NE(nullptr,m_bar);

    QCompleter *result = m_bar->completer();
    EXPECT_EQ(result, m_bar->urlCompleter);
}

TEST_F(DFMAddressBarTest,get_placeholder_text)
{
    ASSERT_NE(nullptr,m_bar);

    QString result = m_bar->placeholderText();
    EXPECT_EQ(result, m_bar->m_placeholderText);
}

TEST_F(DFMAddressBarTest,set_current_url)
{
    ASSERT_NE(nullptr,m_bar);

    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktop("file://" + desktopPath);
    m_bar->setCurrentUrl(desktop);

    EXPECT_EQ(desktop, m_bar->currentUrl);
}

TEST_F(DFMAddressBarTest,set_current_completer)
{
    ASSERT_NE(nullptr,m_bar);

    QCompleter *completer = m_bar->completer();
    m_bar->setCompleter(completer);
    EXPECT_EQ(m_bar->urlCompleter, completer);
}

TEST_F(DFMAddressBarTest,set_placeholder_text)
{
    ASSERT_NE(nullptr,m_bar);

    QString placeholderTest("test_ut");
    m_bar->setPlaceholderText(placeholderTest);
    EXPECT_EQ(m_bar->m_placeholderText, placeholderTest);
}

TEST_F(DFMAddressBarTest,begin_play_animation)
{
    ASSERT_NE(nullptr,m_bar);

    m_bar->playAnimation();
    ASSERT_NE(nullptr, m_bar->animationSpinner);
    bool result = m_bar->animationSpinner->isPlaying();
    EXPECT_TRUE(result);
}

TEST_F(DFMAddressBarTest,end_play_animation)
{
    ASSERT_NE(nullptr,m_bar);

    m_bar->playAnimation();
    m_bar->stopAnimation();
    EXPECT_EQ(nullptr, m_bar->animationSpinner);
}

TEST_F(DFMAddressBarTest,set_no_show)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->hide();
    EXPECT_EQ(true, m_bar->isHidden());
}

TEST_F(DFMAddressBarTest,call_init_ui)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->initUI();
    EXPECT_NE(nullptr, m_bar->indicator);
    EXPECT_NE(nullptr, m_bar->completerView);
    EXPECT_NE(nullptr, m_bar->animation);
}

TEST_F(DFMAddressBarTest,call_init_connections)
{
    ASSERT_NE(nullptr,m_bar);
    emit m_bar->indicator->triggered();

    m_bar->setText("");
    emit m_bar->returnPressed();
    m_bar->setText("dfmvault:/home");
    emit m_bar->returnPressed();

    emit m_bar->selectionChanged();
}

TEST_F(DFMAddressBarTest,set_in_dicator)
{
    ASSERT_NE(nullptr,m_bar);
    DFMAddressBar::IndicatorType type = DFMAddressBar::Search;
    m_bar->setIndicator(type);
    EXPECT_EQ(m_bar->indicatorType, type);

    type = DFMAddressBar::JumpTo;
    m_bar->setIndicator(type);
    EXPECT_EQ(m_bar->indicatorType, type);
}

TEST_F(DFMAddressBarTest,update_indicator_icon)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->updateIndicatorIcon();
    EXPECT_NE(nullptr, m_bar->indicator);
}

TEST_F(DFMAddressBarTest,call_do_complete)
{
    ASSERT_NE(nullptr,m_bar);

    Stub stub;
    void (*ut_showMe)() = [](){};
    stub.set(ADDR(DCompleterListView, showMe), ut_showMe);

    m_bar->doComplete();
    EXPECT_NE(nullptr, m_bar->completerView);
}

TEST_F(DFMAddressBarTest,clear_completer_model)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->clearCompleterModel();
    EXPECT_FALSE(m_bar->isHistoryInCompleterModel);
}

/*
TEST_F(DFMAddressBarTest,update_completion_state)
{
    ASSERT_NE(nullptr,m_bar);
    Stub stub;
    void (*ut_setStringList)() = [](){};
    stub.set(ADDR(QStringListModel, setStringList), ut_setStringList);

    void (*ut_doComplete)() = [](){};
    stub.set(ADDR(DFMAddressBar, doComplete), ut_doComplete);

    DAbstractFileInfoPointer (*createFileInfolamda)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &){
        return DAbstractFileInfoPointer();
    };
    stub.set(ADDR(DFileService,createFileInfo),createFileInfolamda);

    void (*ut_setIndicator)() = [](){};
    stub.set(ADDR(DFMAddressBar, setIndicator), ut_setIndicator);

    void (*ut_completionModelCountChanged)() = [](){};
    stub.set(ADDR(DFMAddressBar, onCompletionModelCountChanged), ut_completionModelCountChanged);

    void (*ut_clearCompleterModel)() = [](){};
    stub.set(ADDR(DFMAddressBar, clearCompleterModel), ut_clearCompleterModel);

    void (*ut_setCompletionPrefix)() = [](){};
    stub.set(ADDR(QCompleter, setCompletionPrefix), ut_setCompletionPrefix);

    m_bar->historyList.clear();

    DFMCrumbBar bar;
    m_bar->setParent(&bar);
    m_bar->updateCompletionState(QString(""));
    m_bar->updateCompletionState(QString("/home"));
    m_bar = nullptr;
}*/

TEST_F(DFMAddressBarTest,append_complete_model)
{
    ASSERT_NE(nullptr,m_bar);

    Stub stub;
    bool (*ut_insertRow)() = [](){return true;};
    stub.set(ADDR(QAbstractItemModel, insertRow), ut_insertRow);

    static bool isCallSetData = false;
    void (*ut_setData)() = [](){isCallSetData = true;};

    typedef bool (*fptr)(QStringListModel*,const QModelIndex&, const QVariant&, int);
    fptr mode_setData = (fptr)(&QStringListModel::setData);
    stub.set(mode_setData, ut_setData);

    QStringList stringList;
//    stringList << QString("test1") << QString("test2");
    m_bar->appendToCompleterModel(stringList);
    EXPECT_FALSE(isCallSetData);
}

TEST_F(DFMAddressBarTest,tst_insert_completion)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->insertCompletion(QString(""));

    m_bar->urlCompleter->setWidget(m_bar);
    m_bar->insertCompletion(QString("/"));
}

TEST_F(DFMAddressBarTest,tst_completion_highlighted)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->onCompletionHighlighted(QString(""));
}

TEST_F(DFMAddressBarTest,tst_completion_modelCount_changed)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->onCompletionModelCountChanged();
}

TEST_F(DFMAddressBarTest,tst_text_edited)
{
    ASSERT_NE(nullptr,m_bar);

    DFMCrumbBar bar;
    m_bar->setParent(&bar);

    m_bar->onTextEdited(QString(""));
    m_bar->onTextEdited(QString("/"));
    m_bar->setParent(nullptr);
}

TEST_F(DFMAddressBarTest,focus_InOut_event)
{
    ASSERT_NE(nullptr,m_bar);

    QFocusEvent eventIn(QEvent::FocusIn);
    m_bar->focusInEvent(&eventIn);

    QFocusEvent eventOut1(QEvent::FocusOut, Qt::OtherFocusReason);
    m_bar->focusOutEvent(&eventOut1);

    QFocusEvent eventOut2(QEvent::FocusOut, Qt::NoFocusReason);
    m_bar->focusOutEvent(&eventOut2);
}

TEST_F(DFMAddressBarTest,key_press_event)
{
    ASSERT_NE(nullptr,m_bar);

    Stub stub;
    static bool myVisible = true;
    bool (*ut_isVisible)() = [](){return myVisible;};
    stub.set(ADDR(QWidget, isVisible), ut_isVisible);

    bool (*ut_removeSearchHistory)() = [](){return true;};
    stub.set(ADDR(SearchHistroyManager, removeSearchHistory), ut_removeSearchHistory);

    void (*ut_setStringList)() = [](){};
    stub.set(ADDR(QStringListModel, setStringList), ut_setStringList);

    QKeyEvent keyPressEvt_Esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_bar->keyPressEvent(&keyPressEvt_Esc);

    bool tempModel = m_bar->isHistoryInCompleterModel;
    m_bar->isHistoryInCompleterModel = true;
    QKeyEvent keyPressEvt_Del(QEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    m_bar->keyPressEvent(&keyPressEvt_Del);
    m_bar->isHistoryInCompleterModel = tempModel;
    stub.reset(ADDR(SearchHistroyManager, removeSearchHistory));
    stub.reset(ADDR(QStringListModel, setStringList));

    QKeyEvent keyPressEvt_BackTab(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_bar->keyPressEvent(&keyPressEvt_BackTab);

    QKeyEvent keyPressEvt_Return(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    m_bar->keyPressEvent(&keyPressEvt_Return);

    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_bar->keyPressEvent(&keyPressEvt_Tab);

    QKeyEvent keyPressEvt_Down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    m_bar->keyPressEvent(&keyPressEvt_Down);

    myVisible = false;
    m_bar->keyPressEvent(&keyPressEvt_Tab);
    m_bar->keyPressEvent(&keyPressEvt_Return);
    m_bar->keyPressEvent(&keyPressEvt_Del);
}

TEST_F(DFMAddressBarTest,tst_paint_event)
{
    ASSERT_NE(nullptr,m_bar);

    QPaintEvent event(QRect(0, 0, 200, 200));
    m_bar->setText("");
    m_bar->playAnimation();
    m_bar->paintEvent(&event);
}

TEST_F(DFMAddressBarTest,tst_show_event)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->show();
}

TEST_F(DFMAddressBarTest,input_method_event)
{
    ASSERT_NE(nullptr,m_bar);

    Stub stub;
    bool (*ut_hasSelectedText)() = [](){return true;};
    stub.set(ADDR(QLineEdit, hasSelectedText), ut_hasSelectedText);

    QInputMethodEvent event;
    m_bar->inputMethodEvent(&event);

}
