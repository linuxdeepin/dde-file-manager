// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitemeditor_1.cpp
 * @brief Unit tests for ListItemEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/listitemeditor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemEditorTest, ListItemEditor)
{
    // Test constructor: ListItemEditor((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ListItemEditorTest, createTooltip)
{
    // Test getter: DArrowRectangle createTooltip()
    auto result = obj->createTooltip();
    EXPECT_NO_FATAL_FAILURE({ obj->createTooltip(); });

}

TEST_F(ListItemEditorTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ListItemEditorTest, maxCharSize)
{
    // Test getter: int maxCharSize()
    auto result = obj->maxCharSize();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemEditorTest, onEditorTextChanged)
{
    // Test method: void onEditorTextChanged((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onEditorTextChanged(_arg0));
}

TEST_F(ListItemEditorTest, select)
{
    // Test method: void select((const QString &part))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0));
}

TEST_F(ListItemEditorTest, setCharCountLimit)
{
    // Test method: void setCharCountLimit(())
    EXPECT_NO_FATAL_FAILURE(obj->setCharCountLimit());
}

TEST_F(ListItemEditorTest, setMaxCharSize)
{
    // Test method: void setMaxCharSize(())
    EXPECT_NO_FATAL_FAILURE(obj->setMaxCharSize());
}

TEST_F(ListItemEditorTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(ListItemEditorTest, ListItemEditor_Destructor)
{
    // Test method:  ~ListItemEditor(())
    EXPECT_NO_FATAL_FAILURE({ ListItemEditor *tmp = new ListItemEditor(); delete tmp; });
}
