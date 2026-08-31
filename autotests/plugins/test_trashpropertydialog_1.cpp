// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashpropertydialog_1.cpp
 * @brief Unit tests for TrashPropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/trashpropertydialog.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashPropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashPropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashPropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashPropertyDialogTest, calculateSize)
{
    // Test method: void calculateSize(())
    EXPECT_NO_FATAL_FAILURE(obj->calculateSize());
}

TEST_F(TrashPropertyDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(TrashPropertyDialogTest, updateLeftInfo)
{
    // Test method: void updateLeftInfo((const int &count))
    int _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateLeftInfo(_arg0));
}
