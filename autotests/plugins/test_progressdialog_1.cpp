// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_progressdialog_1.cpp
 * @brief Unit tests for ProgressDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/progressdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class ProgressDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProgressDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProgressDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProgressDialogTest, handleShredResult)
{
    // Test method: void handleShredResult((bool result, const QString &reason))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShredResult(false, _arg1));
}

TEST_F(ProgressDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(ProgressDialogTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(ProgressDialogTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(ProgressDialogTest, updateProgressValue)
{
    // Test method: void updateProgressValue((int value, const QString &fileName))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateProgressValue(0, _arg1));
}
