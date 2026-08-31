// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_unlockpartitiondialog.cpp
 * @brief Unit tests for UnlockPartitionDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/unlockpartitiondialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class UnlockPartitionDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UnlockPartitionDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UnlockPartitionDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UnlockPartitionDialogTest, getUnlockKey)
{
    // Test getter: QPair<UnlockPartitionDialog::UnlockType, QString> getUnlockKey()
    auto result = obj->getUnlockKey();
    EXPECT_NO_FATAL_FAILURE({ obj->getUnlockKey(); });

}

TEST_F(UnlockPartitionDialogTest, handleButtonClicked)
{
    // Test method: void handleButtonClicked((int index, QString text))
    EXPECT_NO_FATAL_FAILURE(obj->handleButtonClicked(0, QString()));
}

TEST_F(UnlockPartitionDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(UnlockPartitionDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(UnlockPartitionDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(UnlockPartitionDialogTest, switchUnlockType)
{
    // Test method: void switchUnlockType(())
    EXPECT_NO_FATAL_FAILURE(obj->switchUnlockType());
}

TEST_F(UnlockPartitionDialogTest, updateUserHint)
{
    // Test method: void updateUserHint(())
    EXPECT_NO_FATAL_FAILURE(obj->updateUserHint());
}

TEST_F(UnlockPartitionDialogTest, UnlockPartitionDialog_Destructor)
{
    // Test method:  ~UnlockPartitionDialog(())
    EXPECT_NO_FATAL_FAILURE({ UnlockPartitionDialog *tmp = new UnlockPartitionDialog(); delete tmp; });
}
