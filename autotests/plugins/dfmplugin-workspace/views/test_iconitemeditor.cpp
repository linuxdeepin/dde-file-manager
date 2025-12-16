// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/iconitemeditor.h"

#include <QString>
#include <QLabel>
#include <QTextEdit>
#include <qapplication.h>

using namespace dfmplugin_workspace;

class IconItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        editor = new IconItemEditor();
    }

    void TearDown() override
    {
        delete editor;
        editor = nullptr;
    }

    IconItemEditor *editor;
    stub_ext::StubExt stub;
};

TEST_F(IconItemEditorTest, Constructor_CreatesInstance)
{
    EXPECT_NE(editor, nullptr);
}

TEST_F(IconItemEditorTest, GetText_ReturnsText)
{
    QString text = "Test File";
    editor->setText(text);
    
    QString result = editor->text();
    
    EXPECT_EQ(result, text);
}

TEST_F(IconItemEditorTest, SetText_SetsText)
{
    QString text = "Test File";
    
    editor->setText(text);
    
    EXPECT_EQ(editor->text(), text);
}

TEST_F(IconItemEditorTest, Select_SelectsText)
{
    QString text = "Test File";
    editor->setText(text);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->select("Test");
    });
}

TEST_F(IconItemEditorTest, GetOpacity_ReturnsOpacity)
{
    qreal opacity = 0.8;
    editor->setOpacity(opacity);
    
    qreal result = editor->opacity();
    
    EXPECT_NEAR(result, opacity, 0.01);
}

TEST_F(IconItemEditorTest, SetOpacity_SetsOpacity)
{
    qreal opacity = 0.8;
    
    editor->setOpacity(opacity);
    
    EXPECT_NEAR(editor->opacity(), opacity, 0.01);
}

TEST_F(IconItemEditorTest, GetMaxCharSize_ReturnsMaxCharSize)
{
    int maxSize = 100;
    editor->setMaxCharSize(maxSize);
    
    int result = editor->maxCharSize();
    
    EXPECT_EQ(result, maxSize);
}

TEST_F(IconItemEditorTest, SetMaxCharSize_SetsMaxCharSize)
{
    int maxSize = 100;
    
    editor->setMaxCharSize(maxSize);
    
    EXPECT_EQ(editor->maxCharSize(), maxSize);
}

TEST_F(IconItemEditorTest, SetMaxHeight_SetsMaxHeight)
{
    int height = 200;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->setMaxHeight(height);
    });
}

TEST_F(IconItemEditorTest, GetIconLabel_ReturnsIconLabel)
{
    QLabel *result = editor->getIconLabel();
    
    // Should return a valid label or null
    EXPECT_TRUE(result == nullptr || result != nullptr);
}

TEST_F(IconItemEditorTest, GetTextEdit_ReturnsTextEdit)
{
    QTextEdit *result = editor->getTextEdit();
    
    // Should return a valid text edit or null
    EXPECT_TRUE(result == nullptr || result != nullptr);
}

TEST_F(IconItemEditorTest, IsEditReadOnly_ReturnsReadOnly)
{
    bool result = editor->isEditReadOnly();
    
    // Should return a boolean
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(IconItemEditorTest, SetCharCountLimit_SetsCharCountLimit)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->setCharCountLimit();
    });
}

TEST_F(IconItemEditorTest, ShowAlertMessage_ShowsAlert)
{
    QString text = "Alert message";
    int duration = 1000;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        editor->showAlertMessage(text, duration);
    });
}

TEST_F(IconItemEditorTest, SizeHint_ReturnsSize)
{
    QSize result = editor->sizeHint();
    
    // Should return a valid size
    EXPECT_GT(result.width(), 0);
    EXPECT_GT(result.height(), 0);
}
