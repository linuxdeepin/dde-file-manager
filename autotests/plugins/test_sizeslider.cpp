// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sizeslider.cpp
 * @brief Unit tests for SizeSlider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/sizeslider.h"

#include <QTest>

using namespace ddplugin_organizer;

class SizeSliderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SizeSlider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SizeSlider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SizeSliderTest, SizeSlider)
{
    // Test constructor: SizeSlider((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SizeSliderTest, resetToIcon)
{
    // Test method: void resetToIcon(())
    EXPECT_NO_FATAL_FAILURE(obj->resetToIcon());
}

TEST_F(SizeSliderTest, ticks)
{
    // Test method: QStringList ticks((int count))
    auto result = obj->ticks(0);
    EXPECT_TRUE(result.isEmpty());

}
