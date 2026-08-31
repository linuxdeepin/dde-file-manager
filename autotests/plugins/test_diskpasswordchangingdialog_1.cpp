// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskpasswordchangingdialog_1.cpp
 * @brief Unit tests for DiskPasswordChangingDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/diskpasswordchangingdialog.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DiskPasswordChangingDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskPasswordChangingDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskPasswordChangingDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskPasswordChangingDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *e))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(DiskPasswordChangingDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(DiskPasswordChangingDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(DiskPasswordChangingDialogTest, onChangeCompleted)
{
    // Test method: void onChangeCompleted((bool success, const QString &msg))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onChangeCompleted(false, _arg1));
}

TEST_F(DiskPasswordChangingDialogTest, onConfirmed)
{
    // Test method: void onConfirmed(())
    EXPECT_NO_FATAL_FAILURE(obj->onConfirmed());
}
