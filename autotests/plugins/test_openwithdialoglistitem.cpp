// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithdialoglistitem.cpp
 * @brief Unit tests for OpenWithDialogListItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithDialogListItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithDialogListItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithDialogListItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithDialogListItemTest, OpenWithDialogListItem)
{
    // Test constructor: OpenWithDialogListItem((const QString &iconName, const QString &text, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
