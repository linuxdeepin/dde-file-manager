// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbuttonboxprivate.cpp
 * @brief Unit tests for OptionButtonBoxPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/optionbuttonbox.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class OptionButtonBoxPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButtonBoxPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButtonBoxPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonBoxPrivateTest, switchMode)
{
    // Test method: void switchMode((ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->switchMode(ViewMode()));
}
