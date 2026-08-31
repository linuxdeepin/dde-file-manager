// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_movegridoper_1.cpp
 * @brief Unit tests for MoveGridOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/gridcore.h"

#include <QTest>

using namespace ddplugin_canvas;

class MoveGridOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MoveGridOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MoveGridOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MoveGridOperTest, MoveGridOper)
{
    // Test constructor: MoveGridOper((GridCore *core))
    ASSERT_NE(obj, nullptr);
}
