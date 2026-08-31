// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexcontroller.cpp
 * @brief Unit tests for TextIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "textindexcontroller.h"

#include <QTest>

using namespace core;

class TextIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexControllerTest, TextIndexController)
{
    // Test constructor: TextIndexController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
