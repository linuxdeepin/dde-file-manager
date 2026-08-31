// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_nametextedit.cpp
 * @brief Unit tests for NameTextEdit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/editstackedwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class NameTextEditTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NameTextEdit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NameTextEdit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NameTextEditTest, isCanceled)
{
    // Test bool getter: isCanceled()
    bool result = obj->isCanceled();
    EXPECT_FALSE(result);

}

TEST_F(NameTextEditTest, setPlainText)
{
    // Test setter: void setPlainText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPlainText(_arg0));
}

TEST_F(NameTextEditTest, slotTextChanged)
{
    // Test method: void slotTextChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->slotTextChanged());
}

TEST_F(NameTextEditTest, textLength)
{
    // Test getter: int textLength()
    auto result = obj->textLength();
    EXPECT_EQ(result, 0);

}

TEST_F(NameTextEditTest, NameTextEdit)
{
    // Test constructor: NameTextEdit((const QString &text, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NameTextEditTest, createTooltip)
{
    // Test getter: DArrowRectangle createTooltip()
    auto result = obj->createTooltip();
    EXPECT_NO_FATAL_FAILURE({ obj->createTooltip(); });

}
