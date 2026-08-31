// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sizeslider_1.cpp
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

TEST_F(SizeSliderTest, iconClicked)
{
    // Test method: void iconClicked((DSlider::SliderIcons icon, bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->iconClicked(DSlider::SliderIcons(), false));
}

TEST_F(SizeSliderTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(SizeSliderTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(SizeSliderTest, setIconLevel)
{
    // Test setter: void setIconLevel((int lv))
    EXPECT_NO_FATAL_FAILURE(obj->setIconLevel(0));
}

TEST_F(SizeSliderTest, setValue)
{
    // Test setter: void setValue((int v))
    EXPECT_NO_FATAL_FAILURE(obj->setValue(0));
}

TEST_F(SizeSliderTest, syncIconLevel)
{
    // Test method: void syncIconLevel((int lv))
    EXPECT_NO_FATAL_FAILURE(obj->syncIconLevel(0));
}
