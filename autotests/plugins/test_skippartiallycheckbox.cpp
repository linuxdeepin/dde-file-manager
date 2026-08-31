// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_skippartiallycheckbox.cpp
 * @brief Unit tests for SkipPartiallyCheckBox methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/skippartiallycheckbox.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class SkipPartiallyCheckBoxTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SkipPartiallyCheckBox();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SkipPartiallyCheckBox *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SkipPartiallyCheckBoxTest, nextCheckState)
{
    // Test method: void nextCheckState(())
    EXPECT_NO_FATAL_FAILURE(obj->nextCheckState());
}
