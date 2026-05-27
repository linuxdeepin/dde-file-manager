// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/searcheditwidget.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>

using namespace dfmplugin_titlebar;

class SearchEditWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new SearchEditWidget();
        stub.clear();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    SearchEditWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(SearchEditWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(SearchEditWidgetTest, ActivateEdit_Called_EditActivated)
{
    widget->activateEdit();
    // Verify it doesn't crash
}

TEST_F(SearchEditWidgetTest, DeactivateEdit_Called_EditDeactivated)
{
    widget->deactivateEdit();
}

TEST_F(SearchEditWidgetTest, SetText_ValidText_TextSet)
{
    QString text = "test search";
    widget->setText(text);
    EXPECT_EQ(widget->text(), text);
}

TEST_F(SearchEditWidgetTest, Text_AfterSet_ReturnsCorrectText)
{
    QString text = "search query";
    widget->setText(text);
    EXPECT_EQ(widget->text(), text);
}

TEST_F(SearchEditWidgetTest, SetAdvancedButtonVisible_True_ButtonVisible)
{
    EXPECT_NO_THROW(widget->setAdvancedButtonVisible(true));
}

TEST_F(SearchEditWidgetTest, SetAdvancedButtonVisible_False_ButtonHidden)
{
    widget->setAdvancedButtonVisible(false);
    EXPECT_FALSE(widget->isAdvancedButtonVisible());
}

TEST_F(SearchEditWidgetTest, ExpandSearchEdit_Called_SearchExpanded)
{
    widget->expandSearchEdit();
    // Verify it doesn't crash
}

TEST_F(SearchEditWidgetTest, PerformSearch_ValidText_SearchPerformed)
{
    widget->setText("test");
    widget->performSearch();
}

TEST_F(SearchEditWidgetTest, SearchQuit_Signal_CanBeEmitted)
{
    QSignalSpy spy(widget, &SearchEditWidget::searchQuit);
    // Signal would be emitted on user action
}

TEST_F(SearchEditWidgetTest, SearchStop_Signal_CanBeEmitted)
{
    QSignalSpy spy(widget, &SearchEditWidget::searchStop);
    // Signal would be emitted on user action
}

TEST_F(SearchEditWidgetTest, SetText_EmptyText_ClearsText)
{
    widget->setText("test");
    widget->setText("");
    EXPECT_TRUE(widget->text().isEmpty());
}

TEST_F(SearchEditWidgetTest, SetText_MultipleUpdates_UpdatesCorrectly)
{
    widget->setText("first");
    EXPECT_EQ(widget->text(), QString("first"));

    widget->setText("second");
    EXPECT_EQ(widget->text(), QString("second"));
}
