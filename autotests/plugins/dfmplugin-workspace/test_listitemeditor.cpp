// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/listitemeditor.h"

#include <QString>
#include <QLineEdit>
#include <QApplication>

using namespace dfmplugin_workspace;

class ListItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        editor = new ListItemEditor();
        
        // Mock QApplication methods - use static function address
        using FontFunc = QFont(*)();
        stub.set_lamda(static_cast<FontFunc>(&QApplication::font), []() -> QFont {
            return QFont();
        });
    }

    void TearDown() override
    {
        delete editor;
        stub.clear();
    }

    ListItemEditor *editor;
    stub_ext::StubExt stub;
};

TEST_F(ListItemEditorTest, Constructor_CreatesInstance)
{
    EXPECT_NE(editor, nullptr);
}

TEST_F(ListItemEditorTest, GetText_ReturnsText)
{
    QString text = "Test File";
    editor->setText(text);
    
    QString result = editor->text();
    
    EXPECT_EQ(result, text);
}

TEST_F(ListItemEditorTest, SetText_SetsText)
{
    QString text = "Test File";
    
    editor->setText(text);
    
    EXPECT_EQ(editor->text(), text);
}

TEST_F(ListItemEditorTest, Select_SelectsText)
{
    QString text = "Test File";
    editor->setText(text);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->select("Test");
    });
}

TEST_F(ListItemEditorTest, SetMaxCharSize_SetsMaxCharSize)
{
    int maxSize = 100;
    
    editor->setMaxCharSize(maxSize);
    
    EXPECT_EQ(editor->maxCharSize(), maxSize);
}

TEST_F(ListItemEditorTest, GetMaxCharSize_ReturnsMaxCharSize)
{
    int maxSize = 100;
    editor->setMaxCharSize(maxSize);
    
    int result = editor->maxCharSize();
    
    EXPECT_EQ(result, maxSize);
}

TEST_F(ListItemEditorTest, SetCharCountLimit_SetsCharCountLimit)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->setCharCountLimit();
    });
}

TEST_F(ListItemEditorTest, ShowAlertMessage_ShowsAlert)
{
    QString text = "Alert message";
    int duration = 1000;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->showAlertMessage(text, duration);
    });
}

TEST_F(ListItemEditorTest, Event_HandlesEvent)
{
    QEvent event(QEvent::None);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        bool result = editor->event(&event);
        (void)result; // Suppress unused variable warning
    });
}
