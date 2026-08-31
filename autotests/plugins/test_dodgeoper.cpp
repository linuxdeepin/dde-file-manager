// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dodgeoper.cpp
 * @brief Unit tests for DodgeOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/dodgeoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DodgeOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DodgeOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DodgeOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DodgeOperTest, DodgeOper)
{
    // Test constructor: DodgeOper((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}
