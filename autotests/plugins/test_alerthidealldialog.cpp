// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_alerthidealldialog.cpp
 * @brief Unit tests for AlertHideAllDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/alerthidealldialog.h"

#include <QTest>

using namespace ddplugin_organizer;

class AlertHideAllDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AlertHideAllDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AlertHideAllDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AlertHideAllDialogTest, confirmBtnIndex)
{
    // Test getter: int confirmBtnIndex()
    auto result = obj->confirmBtnIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(AlertHideAllDialogTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *o, QEvent *e))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AlertHideAllDialogTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(AlertHideAllDialogTest, isRepeatNoMore)
{
    // Test bool getter: isRepeatNoMore()
    bool result = obj->isRepeatNoMore();
    EXPECT_FALSE(result);

}
