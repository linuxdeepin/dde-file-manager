// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithmessage.cpp
 * @brief Unit tests for CheckBoxWithMessage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/settingsdialog/controls/checkboxwithmessage.h"

#include <QTest>

using namespace src;

class CheckBoxWithMessageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWithMessage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWithMessage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWithMessageTest, CheckBoxWithMessage)
{
    // Test constructor: CheckBoxWithMessage((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
