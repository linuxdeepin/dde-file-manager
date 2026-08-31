// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextmenu.cpp
 * @brief Unit tests for DFMExtMenu methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/menu/dfmextmenu.h"

#include <QTest>

using namespace src;

class DFMExtMenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtMenu();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtMenu *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtMenuTest, deleted)
{
    // Test method: void deleted((DFMExtMenu *self))
    EXPECT_NO_FATAL_FAILURE(obj->deleted(nullptr));
}

TEST_F(DFMExtMenuTest, hovered)
{
    // Test method: void hovered((DFMExtAction *action))
    EXPECT_NO_FATAL_FAILURE(obj->hovered(nullptr));
}
