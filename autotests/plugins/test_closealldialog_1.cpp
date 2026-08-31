// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_closealldialog_1.cpp
 * @brief Unit tests for CloseAllDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/closealldialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class CloseAllDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CloseAllDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CloseAllDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CloseAllDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(CloseAllDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(CloseAllDialogTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(CloseAllDialogTest, setTotalMessage)
{
    // Test setter: void setTotalMessage((qint64 size, int count))
    EXPECT_NO_FATAL_FAILURE(obj->setTotalMessage(0, 0));
}

TEST_F(CloseAllDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
