// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridspecialist_1.cpp
 * @brief Unit tests for CanvasGridSpecialist methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgridspecialist.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridSpecialistTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridSpecialist();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridSpecialist *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridSpecialistTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridSpecialistTest, profileIndex)
{
    // Test method: int profileIndex((QString screenKey))
    auto result = obj->profileIndex(QString());
    EXPECT_GE(result, 0);

}

TEST_F(CanvasGridSpecialistTest, profileKey)
{
    // Test method: QString profileKey((int index))
    auto result = obj->profileKey(0);
    EXPECT_TRUE(result.isEmpty());

}
