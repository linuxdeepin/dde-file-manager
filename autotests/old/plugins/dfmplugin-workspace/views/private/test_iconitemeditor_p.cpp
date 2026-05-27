// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/private/iconitemeditor_p.h"
#include "views/iconitemeditor.h"

#include <QLabel>
#include <QTextEdit>
#include <QStack>
#include <QGraphicsOpacityEffect>
#include <DArrowRectangle>

using namespace dfmplugin_workspace;

class IconItemEditorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        editor = new IconItemEditor();
        d = new IconItemEditorPrivate(editor);
    }

    void TearDown() override
    {
        delete d;
        delete editor;
        stub.clear();
    }

    IconItemEditor *editor = nullptr;
    IconItemEditorPrivate *d = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemEditorPrivateTest, Constructor_SetsQPointer)
{
    // Test that constructor sets q_ptr correctly
    EXPECT_EQ(d->q_ptr, editor);
}

TEST_F(IconItemEditorPrivateTest, Destructor_DoesNotCrash)
{
    // Test destructor
    auto *testD = new IconItemEditorPrivate(editor);
    EXPECT_NO_THROW(delete testD);
}

TEST_F(IconItemEditorPrivateTest, CanDeferredDelete_InitialValue)
{
    // Test initial value of canDeferredDelete
    EXPECT_TRUE(d->canDeferredDelete);
}

TEST_F(IconItemEditorPrivateTest, Icon_InitialValue)
{
    // Test initial value of icon
    EXPECT_EQ(d->icon, nullptr);
}

TEST_F(IconItemEditorPrivateTest, Edit_InitialValue)
{
    // Test initial value of edit
    EXPECT_EQ(d->edit, nullptr);
}

TEST_F(IconItemEditorPrivateTest, EditTextStackCurrentIndex_InitialValue)
{
    // Test initial value of editTextStackCurrentIndex
    EXPECT_EQ(d->editTextStackCurrentIndex, -1);
}

TEST_F(IconItemEditorPrivateTest, DisableEditTextStack_InitialValue)
{
    // Test initial value of disableEditTextStack
    EXPECT_FALSE(d->disableEditTextStack);
}

TEST_F(IconItemEditorPrivateTest, MaxCharSize_InitialValue)
{
    // Test initial value of maxCharSize
    EXPECT_EQ(d->maxCharSize, INT_MAX);
}

TEST_F(IconItemEditorPrivateTest, MaxHeight_InitialValue)
{
    // Test initial value of maxHeight
    EXPECT_EQ(d->maxHeight, -1);
}

TEST_F(IconItemEditorPrivateTest, UseCharCountLimit_InitialValue)
{
    // Test initial value of useCharCountLimit
    EXPECT_FALSE(d->useCharCountLimit);
}

TEST_F(IconItemEditorPrivateTest, Tooltip_InitialValue)
{
    // Test initial value of tooltip
    EXPECT_EQ(d->tooltip, nullptr);
}

TEST_F(IconItemEditorPrivateTest, ValidText_InitialValue)
{
    // Test initial value of validText
    EXPECT_TRUE(d->validText.isEmpty());
}

TEST_F(IconItemEditorPrivateTest, SetCanDeferredDelete_ValidValue_SetsValue)
{
    // Test setting canDeferredDelete
    bool testValue = false;
    d->canDeferredDelete = testValue;
    
    EXPECT_EQ(d->canDeferredDelete, testValue);
}

TEST_F(IconItemEditorPrivateTest, SetEditTextStackCurrentIndex_ValidIndex_SetsIndex)
{
    // Test setting editTextStackCurrentIndex
    int testIndex = 5;
    d->editTextStackCurrentIndex = testIndex;
    
    EXPECT_EQ(d->editTextStackCurrentIndex, testIndex);
}

TEST_F(IconItemEditorPrivateTest, SetDisableEditTextStack_ValidValue_SetsValue)
{
    // Test setting disableEditTextStack
    bool testValue = true;
    d->disableEditTextStack = testValue;
    
    EXPECT_EQ(d->disableEditTextStack, testValue);
}

TEST_F(IconItemEditorPrivateTest, SetMaxCharSize_ValidSize_SetsSize)
{
    // Test setting maxCharSize
    int testSize = 100;
    d->maxCharSize = testSize;
    
    EXPECT_EQ(d->maxCharSize, testSize);
}

TEST_F(IconItemEditorPrivateTest, SetMaxHeight_ValidHeight_SetsHeight)
{
    // Test setting maxHeight
    int testHeight = 200;
    d->maxHeight = testHeight;
    
    EXPECT_EQ(d->maxHeight, testHeight);
}

TEST_F(IconItemEditorPrivateTest, SetUseCharCountLimit_ValidValue_SetsValue)
{
    // Test setting useCharCountLimit
    bool testValue = true;
    d->useCharCountLimit = testValue;
    
    EXPECT_EQ(d->useCharCountLimit, testValue);
}

TEST_F(IconItemEditorPrivateTest, SetValidText_ValidText_SetsText)
{
    // Test setting validText
    QString testText("test_valid_text");
    d->validText = testText;
    
    EXPECT_EQ(d->validText, testText);
}

TEST_F(IconItemEditorPrivateTest, EditTextStack_InitialValue)
{
    // Test initial value of editTextStack
    EXPECT_TRUE(d->editTextStack.isEmpty());
}

TEST_F(IconItemEditorPrivateTest, OpacityEffect_InitialValue)
{
    // Test initial value of opacityEffect
    EXPECT_EQ(d->opacityEffect, nullptr);
}

TEST_F(IconItemEditorPrivateTest, SetOpacityEffect_ValidEffect_SetsEffect)
{
    // Test setting opacityEffect
    auto testEffect = new QGraphicsOpacityEffect();
    d->opacityEffect = testEffect;
    
    EXPECT_EQ(d->opacityEffect, testEffect);
    
    delete testEffect;
}

TEST_F(IconItemEditorPrivateTest, EditTextStack_AddText_AddsToStack)
{
    // Test adding text to editTextStack
    QString testText("test_text");
    d->editTextStack.push(testText);
    
    EXPECT_FALSE(d->editTextStack.isEmpty());
    EXPECT_EQ(d->editTextStack.top(), testText);
}

TEST_F(IconItemEditorPrivateTest, EditTextStack_ClearStack_ClearsStack)
{
    // Test clearing editTextStack
    d->editTextStack.push("test1");
    d->editTextStack.push("test2");
    
    d->editTextStack.clear();
    
    EXPECT_TRUE(d->editTextStack.isEmpty());
}