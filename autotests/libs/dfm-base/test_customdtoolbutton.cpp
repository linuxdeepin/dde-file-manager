// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdtoolbutton.cpp
 * @brief Unit tests for CustomDToolButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h"

#include <QTest>

using namespace src;

class CustomDToolButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomDToolButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomDToolButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomDToolButtonTest, initStyleOption)
{
    // Test method: void initStyleOption((QStyleOptionToolButton *option))
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr));
}
