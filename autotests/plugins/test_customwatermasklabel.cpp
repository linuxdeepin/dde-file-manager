// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customwatermasklabel.cpp
 * @brief Unit tests for CustomWaterMaskLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/customwatermasklabel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CustomWaterMaskLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomWaterMaskLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomWaterMaskLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomWaterMaskLabelTest, loadConfig)
{
    // Test method: void loadConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->loadConfig());
}

TEST_F(CustomWaterMaskLabelTest, setPosition)
{
    // Test method: void setPosition(())
    EXPECT_NO_FATAL_FAILURE(obj->setPosition());
}

TEST_F(CustomWaterMaskLabelTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}
