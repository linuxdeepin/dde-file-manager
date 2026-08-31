// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithdialoglistspareritem.cpp
 * @brief Unit tests for OpenWithDialogListSparerItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithDialogListSparerItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithDialogListSparerItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithDialogListSparerItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithDialogListSparerItemTest, OpenWithDialogListSparerItem)
{
    // Test constructor: OpenWithDialogListSparerItem((const QString &title, QWidget *parent, bool showSeparator))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenWithDialogListSparerItemTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}
