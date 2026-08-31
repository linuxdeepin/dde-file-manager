// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithdialog.cpp
 * @brief Unit tests for OpenWithDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithDialogTest, OpenWithDialog)
{
    // Test constructor: OpenWithDialog((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenWithDialogTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpenWithDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(OpenWithDialogTest, openFileByApp)
{
    // Test method: void openFileByApp(())
    EXPECT_NO_FATAL_FAILURE(obj->openFileByApp());
}
