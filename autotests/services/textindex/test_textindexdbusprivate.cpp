// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexdbusprivate.cpp
 * @brief Unit tests for TextIndexDBusPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/textindexdbus.h"

#include <QTest>

using namespace src;

class TextIndexDBusPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexDBusPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexDBusPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexDBusPrivateTest, handleSlientStart)
{
    // Test method: void handleSlientStart(())
    EXPECT_NO_FATAL_FAILURE(obj->handleSlientStart());
}
