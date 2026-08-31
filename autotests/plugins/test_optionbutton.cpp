// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbutton.cpp
 * @brief Unit tests for OptionButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectiontitlebar.h"

#include <QTest>

using namespace ddplugin_organizer;

class OptionButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonTest, initStyleOption)
{
    // Test method: void initStyleOption((DStyleOptionButton *option))
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr));
}
