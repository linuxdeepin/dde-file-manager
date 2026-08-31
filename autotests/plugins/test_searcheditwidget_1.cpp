// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searcheditwidget_1.cpp
 * @brief Unit tests for SearchEditWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/searcheditwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SearchEditWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchEditWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchEditWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchEditWidgetTest, activateEdit)
{
    // Test method: void activateEdit((bool setAdvanceBtn))
    EXPECT_NO_FATAL_FAILURE(obj->activateEdit(false));
}

TEST_F(SearchEditWidgetTest, deactivateEdit)
{
    // Test method: void deactivateEdit(())
    EXPECT_NO_FATAL_FAILURE(obj->deactivateEdit());
}

TEST_F(SearchEditWidgetTest, determineSearchDelay)
{
    // Test method: int determineSearchDelay((const QString &inputText))
    QString _arg0{};
    auto result = obj->determineSearchDelay(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(SearchEditWidgetTest, expandSearchEdit)
{
    // Test method: void expandSearchEdit(())
    EXPECT_NO_FATAL_FAILURE(obj->expandSearchEdit());
}

TEST_F(SearchEditWidgetTest, handleFocusInEvent)
{
    // Test event handler: handleFocusInEvent((QFocusEvent *e))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->handleFocusInEvent(&_event));
}

TEST_F(SearchEditWidgetTest, handleFocusOutEvent)
{
    // Test event handler: handleFocusOutEvent((QFocusEvent *e))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->handleFocusOutEvent(&_event));
}

TEST_F(SearchEditWidgetTest, handleInputMethodEvent)
{
    // Test event handler: handleInputMethodEvent((QInputMethodEvent *e))
    QInputMethodEvent _event(QInputMethodEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->handleInputMethodEvent(&_event));
}

TEST_F(SearchEditWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(SearchEditWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(SearchEditWidgetTest, isAdvancedButtonChecked)
{
    // Test bool getter: isAdvancedButtonChecked()
    bool result = obj->isAdvancedButtonChecked();
    EXPECT_FALSE(result);

}

TEST_F(SearchEditWidgetTest, isAdvancedButtonVisible)
{
    // Test bool getter: isAdvancedButtonVisible()
    bool result = obj->isAdvancedButtonVisible();
    EXPECT_FALSE(result);

}

TEST_F(SearchEditWidgetTest, onAdvancedButtonClicked)
{
    // Test method: void onAdvancedButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onAdvancedButtonClicked());
}

TEST_F(SearchEditWidgetTest, onTextEdited)
{
    // Test method: void onTextEdited((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTextEdited(_arg0));
}

TEST_F(SearchEditWidgetTest, performSearch)
{
    // Test method: void performSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->performSearch());
}

TEST_F(SearchEditWidgetTest, quitSearch)
{
    // Test method: void quitSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->quitSearch());
}

TEST_F(SearchEditWidgetTest, setAdvancedButtonChecked)
{
    // Test setter: void setAdvancedButtonChecked((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->setAdvancedButtonChecked(false));
}

TEST_F(SearchEditWidgetTest, setAdvancedButtonVisible)
{
    // Test setter: void setAdvancedButtonVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setAdvancedButtonVisible(false));
}

TEST_F(SearchEditWidgetTest, setSearchMode)
{
    // Test setter: void setSearchMode((SearchMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setSearchMode(SearchMode()));
}

TEST_F(SearchEditWidgetTest, setText)
{
    // Test setter: void setText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(SearchEditWidgetTest, shouldDelaySearch)
{
    // Test method: bool shouldDelaySearch((const QString &inputText))
    QString _arg0{};
    auto result = obj->shouldDelaySearch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SearchEditWidgetTest, stopSearch)
{
    // Test method: void stopSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->stopSearch());
}

TEST_F(SearchEditWidgetTest, updateSearchEditWidget)
{
    // Test method: void updateSearchEditWidget((int parentWidth))
    EXPECT_NO_FATAL_FAILURE(obj->updateSearchEditWidget(0));
}

TEST_F(SearchEditWidgetTest, updateSearchWidgetLayout)
{
    // Test method: void updateSearchWidgetLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSearchWidgetLayout());
}

TEST_F(SearchEditWidgetTest, updateSpacing)
{
    // Test method: void updateSpacing((bool showAdvancedButton))
    EXPECT_NO_FATAL_FAILURE(obj->updateSpacing(false));
}
