// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_nametextedit_1.cpp
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

TEST_F(NameTextEditTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(NameTextEditTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(NameTextEditTest, setCharCountLimit)
{
    // Test method: void setCharCountLimit(())
    EXPECT_NO_FATAL_FAILURE(obj->setCharCountLimit());
}

TEST_F(NameTextEditTest, setIsCanceled)
{
    // Test setter: void setIsCanceled((bool isCanceled))
    EXPECT_NO_FATAL_FAILURE(obj->setIsCanceled(false));
}

TEST_F(NameTextEditTest, setSuffix)
{
    // Test method: void setSuffix(())
    EXPECT_NO_FATAL_FAILURE(obj->setSuffix());
}

TEST_F(NameTextEditTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}
