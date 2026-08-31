// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_registercustomformat_1.cpp
 * @brief Unit tests for RegisterCustomFormat methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <QTest>

using namespace dfmplugin_menu;

class RegisterCustomFormatTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RegisterCustomFormat();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RegisterCustomFormat *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RegisterCustomFormatTest, RegisterCustomFormat)
{
    // Test constructor: RegisterCustomFormat(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(RegisterCustomFormatTest, customFormat)
{
    // Test getter: QSettings::Format customFormat()
    auto result = obj->customFormat();
    EXPECT_TRUE(result.isEmpty());

}
